#include "agents.h"
// #include "CBS.h"
#include "PBS.h"
#include "genetic.h"
#include "graph.h"

#include "task_assigner.h"

#include "yaml-cpp/yaml.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <set>
#include <unordered_map>

std::vector<Point> GenerateLayout(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "path to data file is not specified" << std::endl;
    exit(0);
  }
  Graph graph(argv[1]);
  const size_t assignments = 100;
  const TaskAssigner task_assigner_init(graph, assignments);
  TaskAssigner task_assigner = task_assigner_init;
  const Agents agents_init(graph, 10);
  Agents agents = agents_init;
  const size_t generation_size = 5;
  Generation generation(
      generation_size, graph.GetEjectCheckpoints().size(), graph.GetInductCheckpoints().size());


  struct Assignment {
    std::vector<std::vector<Point>> paths;
    double throughput;
    std::vector<size_t> eject_checkpoints_permutation;
    std::vector<size_t> induct_checkpoints_permutation;
  };
  double total_throughput = 0.0;
  double min_throughput = std::numeric_limits<double>::max();
  std::optional<Assignment> best_assignment;

  const size_t steps = 10;
  for (size_t i = 0; i < steps; ++i) {
    for (auto& chromosome : generation.GetChromosomesMutable()) {
      task_assigner = task_assigner_init;
      agents = agents_init;
      graph.ApplyPermutation(
          chromosome.eject_checkpoints_permutation, chromosome.induct_checkpoints_permutation);
      auto paths = PriorityBasedSearch(agents, graph, task_assigner, 30);
      const double throughput = CalculateThroughput(paths, assignments);
      chromosome.SetScore(throughput);
      std::cerr << "PF result throughput : " << throughput << std::endl;
      if (!best_assignment || best_assignment->throughput < throughput) {
        if (!best_assignment) {
          best_assignment = Assignment();
        }
        best_assignment->paths = std::move(paths);
        best_assignment->throughput = throughput;
        best_assignment->eject_checkpoints_permutation = chromosome.eject_checkpoints_permutation;
        best_assignment->induct_checkpoints_permutation = chromosome.induct_checkpoints_permutation;
      }
      min_throughput = std::min(min_throughput, throughput);
      total_throughput += throughput;
    }
  }

  assert(best_assignment);
  for (size_t i = 0; i < best_assignment->paths.size(); ++i) {
    const Agent& cur_agent = agents.At(i);
    std::cout << "Path for agent " << cur_agent.id << " : ";
    for (const auto& position : best_assignment->paths[i]) {
      std::cout << position << " ";
    }
    std::cout << std::endl;
    cur_agent.PrintDebugInfo(std::cout);
  }
  std::cout << "Worst throughput : " << min_throughput << std::endl;
  std::cout << "Best throughput : " << best_assignment->throughput << std::endl;
  std::cout << "Average throughput : "
            << total_throughput / (steps * generation_size) << std::endl;

  return {};
}

int main(int argc, char** argv) {
  GenerateLayout(argc, argv);
}
