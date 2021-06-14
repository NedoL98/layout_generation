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
  const TaskAssigner task_assigner_init(graph, 100);
  TaskAssigner task_assigner = task_assigner;
  const Agents agents_init(graph, 10);
  Agents agents = agents_init;
  Generation generation(
      3, graph.GetEjectCheckpoints().size(), graph.GetInductCheckpoints().size());

  for (const auto& chromosome : generation.GetChromosomes()) {
    task_assigner = task_assigner_init;
    agents = agents_init;
    graph.ApplyPermutation(
        chromosome.eject_checkpoints_permutation, chromosome.induct_checkpoints_permutation);
    const auto paths = PriorityBasedSearch(agents, graph, task_assigner, 30);
    /*
    for (size_t i = 0; i < paths.size(); ++i) {
      const Agent& cur_agent = agents.At(i);
      std::cout << "Path for agent " << cur_agent.id << " : ";
      for (const auto& position : paths[i]) {
        std::cout << position << " ";
      }
      std::cout << std::endl;
      cur_agent.PrintDebugInfo(std::cout);
    }
    */
    std::cerr << "PF result cost : " << CalculateCost(paths) << std::endl;
  }
  return {};
}

int main(int argc, char** argv) {
  GenerateLayout(argc, argv);
}
