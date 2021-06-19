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
  Generation generation(
      3, graph.GetEjectCheckpoints().size(), graph.GetInductCheckpoints().size());


  struct Assignment {
    std::vector<std::vector<Point>> paths;
    double cost;
    std::vector<size_t> eject_checkpoints_permutation;
    std::vector<size_t> induct_checkpoints_permutation;
  };
  std::optional<Assignment> best_assignment;

  for (size_t i = 0; i < 1; ++i) {
    for (auto& chromosome : generation.GetChromosomesMutable()) {
      task_assigner = task_assigner_init;
      agents = agents_init;
      graph.ApplyPermutation(
          chromosome.eject_checkpoints_permutation, chromosome.induct_checkpoints_permutation);
      auto paths = PriorityBasedSearch(agents, graph, task_assigner, 30);
      const double cost = CalculateCost(paths);
      chromosome.SetScore(cost);
      std::cerr << "PF result cost : " << cost << std::endl;
      if (!best_assignment || best_assignment->cost > cost) {
        if (!best_assignment) {
          best_assignment = Assignment();
        }
        best_assignment->paths = std::move(paths);
        best_assignment->cost = cost;
        best_assignment->eject_checkpoints_permutation = chromosome.eject_checkpoints_permutation;
        best_assignment->induct_checkpoints_permutation = chromosome.induct_checkpoints_permutation;
      }
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
  std::cout << "Final throughput : "
            << CalculateThroughput(best_assignment->paths, assignments) << std::endl;

  return {};
}

int main(int argc, char** argv) {
  GenerateLayout(argc, argv);
}
