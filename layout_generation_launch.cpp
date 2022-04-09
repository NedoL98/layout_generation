#include "agents.h"
// #include "CBS.h"
#include "PBS.h"
#include "genetic.h"
#include "graph.h"

#include "task_assigner.h"

#include "yaml-cpp/yaml.h"

#include <cassert>
#include <fstream>
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

}

void GenerateLayout(int argc, char** argv) {
  if (argc != 5) {
    std::cerr << "please specify following params: " << std::endl;
    std::cerr << "    - path to data file" << std::endl;
    std::cerr << "    - number of assignments" << std::endl;
    std::cerr << "    - kept eject induct ratio" << std::endl;
    std::cerr << "    - number of epochs" << std::endl;
    exit(0);
  }
  // Mute all cerr
  freopen("log.cerr", "w", stderr);
  Graph graph_full(argv[1], 1.0);
  const size_t assignments_cnt = std::atoi(argv[2]);
  const double kept_checkpoint_ratio = std::stod(argv[3]);
  TaskAssigner task_assigner_init(
      graph_full.GetInductCheckpoints().size() * kept_checkpoint_ratio,
      graph_full.GetEjectCheckpoints().size(),
      assignments_cnt);
  const Agents agents_init(graph_full, 10);
  const size_t generation_size = 3;
  Generation generation(
      generation_size, graph_full.GetInductCheckpoints().size(), kept_checkpoint_ratio);

  double total_throughput = 0.0;
  double min_throughput = std::numeric_limits<double>::max();
  std::optional<BestAssignment> best_assignment;

  std::mutex mtx;
  const auto& run_pbs = [&](Chromosome& chromosome) {
    Graph graph = graph_full;
    graph.KeepOnlySelectedCheckpoints(chromosome.induct_checkpoints_permutation);
    TaskAssigner task_assigner = task_assigner_init;
    Agents agents = agents_init;
    // Explicitly check that
    // - graph is connected
    // - it's possible to reach all the eject checkpoints
    if (!graph.IsConnected() || !graph.AllInductCheckpointsAreReachable()) {
      chromosome.SetScore(std::numeric_limits<double>::max());
      return;
    }
    auto paths = PriorityBasedSearch(agents, graph, task_assigner, 30);
    const double throughput = CalculateThroughput(paths, assignments_cnt);
    chromosome.SetScore(throughput);
    {
      std::lock_guard<std::mutex> lock(mtx);
      if (!best_assignment || best_assignment->throughput < throughput) {
        if (!best_assignment) {
          best_assignment = BestAssignment();
        }
        best_assignment->paths = std::move(paths);
        best_assignment->throughput = throughput;
        best_assignment->induct_checkpoints_indices = chromosome.induct_checkpoints_permutation;
        best_assignment->graph = graph;
        best_assignment->agents = agents;
      }
      min_throughput = std::min(min_throughput, throughput);
      total_throughput += throughput;
    }
  };

  const size_t steps = std::atoi(argv[4]);
  for (size_t i = 0; i < steps; ++i) {
    std::cout << "Generation " << i + 1 << std::endl;
    std::cout.flush();
    auto chromosomes = generation.GetChromosomesMutable();
    std::vector<std::thread> threads;
    threads.reserve(generation.GetChromosomesMutable().size());
    for (auto& chromosome : generation.GetChromosomesMutable()) {
      threads.push_back(std::thread(run_pbs, std::ref(chromosome)));
    }
    for (auto& thread : threads) {
      thread.join();
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
