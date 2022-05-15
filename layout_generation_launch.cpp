#include "agents.h"
#include "arguments_parser.h"
// #include "CBS.h"
#include "PBS.h"
#include "genetic.h"
#include "graph.h"

#include "task_assigner.h"

#include "yaml-cpp/yaml.h"

#include <fstream>
#include <future>
#include <iostream>
#include <optional>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>

struct BestAssignment {
  std::vector<std::vector<Point>> paths;
  double throughput;
  std::vector<size_t> induct_checkpoints_indices;
  Graph graph;
  Agents agents;
};

namespace {

void LogBestAssignment(const std::optional<BestAssignment>& assignment_opt, const size_t epoch) {
  std::ofstream outfile;
  const std::string filename = "data/best_assignment_epoch_" + std::to_string(epoch);
  outfile.open(filename);
  if (assignment_opt) {
    const auto& assignment = assignment_opt.value();
    for (size_t i = 0; i < assignment.paths.size(); ++i) {
      const Agent& cur_agent = assignment.agents.At(i);
      outfile << "Path for agent " << cur_agent.id << " : ";
      for (const auto& position : assignment.paths[i]) {
        outfile << position << " ";
      }
      outfile << std::endl;
      cur_agent.PrintDebugInfo(outfile);
    }
  }
  outfile.close();
}

struct TaskAssigners {
  TaskAssigners() = delete;
  TaskAssigners(
      const size_t assigners_cnt,
      const size_t induct_cnt,
      const size_t eject_cnt,
      const size_t assignments_cnt) {
    assigners.reserve(assigners_cnt);
    for (size_t i = 0; i < assigners_cnt; ++i) {
      assigners.push_back(TaskAssigner(induct_cnt, eject_cnt, assignments_cnt, i + 1));
    }
  }

  std::vector<TaskAssigner> assigners;
};

}

void GenerateLayout(int argc, char** argv) {
  // Mute all cerr
  freopen("log.cerr", "w", stderr);

  const auto params = ParseArguments(argc, argv);

  Graph graph_full(params["file"].as<std::string>(), 1.0);
  const size_t assignments_cnt = params["assignments"].as<size_t>();
  const double kept_checkpoint_ratio = params["checkpoints_ratio"].as<double>();
  TaskAssigners task_assigners_init(
      params["chains"].as<size_t>(),
      graph_full.GetInductCheckpoints().size() * kept_checkpoint_ratio,
      graph_full.GetEjectCheckpoints().size(),
      assignments_cnt);
  const Agents agents_init(graph_full, params["agents"].as<size_t>());
  const size_t generation_size = 3;
  Generation generation(
      generation_size,
      graph_full.GetInductCheckpoints().size(),
      kept_checkpoint_ratio,
      params["entropy"].as<double>());

  double total_throughput = 0.0;
  double min_throughput = std::numeric_limits<double>::max();
  std::optional<BestAssignment> best_assignment;

  const size_t steps = params["epochs"].as<size_t>();
  for (size_t i = 0; i < steps; ++i) {
    std::cout << "Generation " << i + 1 << std::endl;
    std::cout.flush();
    auto& chromosomes = generation.GetChromosomesMutable();
    std::vector<std::vector<std::thread>> threads;
    std::vector<std::vector<std::future<std::pair<Paths, Agents>>>> futures;
    threads.reserve(chromosomes.size());
    futures.reserve(chromosomes.size());

    for (auto& chromosome : chromosomes) {
      if (threads.size() < chromosomes.size()) {
        threads.emplace_back(std::vector<std::thread>(task_assigners_init.assigners.size()));
        futures.emplace_back(std::vector<std::future<std::pair<Paths, Agents>>>(task_assigners_init.assigners.size()));
      }

      Graph graph = graph_full;
      graph.KeepOnlySelectedCheckpoints(chromosome.GetCheckpointsPermutation());
      // Explicitly check that
      // - graph is connected
      // - it's possible to reach all the eject checkpoints
      if (!graph.IsConnected() || !graph.AllInductCheckpointsAreReachable()) {
        chromosome.Invalidate();
        continue;
      }

      TaskAssigners task_assigners = task_assigners_init;
      for (size_t i = 0; i < task_assigners.assigners.size(); ++i) {
        std::promise<std::pair<Paths, Agents>> promise;
        futures.back()[i] = promise.get_future();
        threads.back()[i] = std::thread(
            PriorityBasedSearch,
            agents_init,
            graph,
            task_assigners.assigners[i],
            30,
            std::move(promise));
      }
    }

    for (size_t i = 0; i < chromosomes.size(); ++i) {
      if (!threads[i].front().joinable()) {
        // Chromosome is invalid
        continue;
      }

      double throughput_avg = 0;
      Paths first_assigner_paths;
      Agents first_assigner_agents;

      for (size_t j = 0; j < task_assigners_init.assigners.size(); ++j) {
        auto [paths, agents] = futures[i][j].get();
        const double throughput = CalculateThroughput(paths, assignments_cnt);
        throughput_avg += throughput;
        if (j == 0) {
          first_assigner_paths = std::move(paths);
          first_assigner_agents = std::move(agents);
        }
      }
      throughput_avg /= task_assigners_init.assigners.size();
      chromosomes[i].SetScore(throughput_avg);

      if (!best_assignment || best_assignment->throughput < throughput_avg) {
        if (!best_assignment) {
          best_assignment = BestAssignment();
        }
        best_assignment->paths = std::move(first_assigner_paths);
        best_assignment->throughput = throughput_avg;
        best_assignment->induct_checkpoints_indices = chromosomes[i].GetCheckpointsPermutation();
        Graph graph = graph_full;
        graph.KeepOnlySelectedCheckpoints(chromosomes[i].GetCheckpointsPermutation());
        best_assignment->graph = std::move(graph);
        best_assignment->agents = std::move(first_assigner_agents);
      }
      min_throughput = std::min(min_throughput, throughput_avg);
      total_throughput += throughput_avg;
    }

    for (auto& assigner_threads : threads) {
      for (auto& thread : assigner_threads) {
        if (thread.joinable()) {
          thread.join();
        }
      }
    }
    std::cout << "Done " << i + 1 << std::endl;
    std::cout.flush();

    if (i % 10 == 0) {
      LogBestAssignment(best_assignment, i);
    }
    generation.Evolve();
  }

  if (!best_assignment) {
    std::cout << "No solution found" << std::endl;
    return;
  }
  LogBestAssignment(best_assignment, steps);
  std::cout << "Worst throughput : " << min_throughput << std::endl;
  std::cout << "Best throughput : " << best_assignment->throughput << std::endl;
  std::cout << "Average throughput : "
            << total_throughput / (steps * generation_size) << std::endl;

  return;
}

int main(int argc, char** argv) {
  GenerateLayout(argc, argv);
}
