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
  if (argc != 5) {
    std::cerr << "please specify following params: " << std::endl;
    std::cerr << "    - path to data file" << std::endl;
    std::cerr << "    - number of assignments" << std::endl;
    std::cerr << "    - deleted eject checkpoint ratio" << std::endl;
    std::cerr << "    - number of epochs" << std::endl;
    exit(0);
  }
  Graph graph_full(argv[1], 1.0);
  const size_t assignments_cnt = std::atoi(argv[2]);
  const double kept_checkpoint_ratio = std::stod(argv[3]);
  TaskAssigner task_assigner_init(
      graph_full.GetInductCheckpoints().size(),
      graph_full.GetEjectCheckpoints().size() * kept_checkpoint_ratio,
      assignments_cnt);
  TaskAssigner task_assigner = task_assigner_init;
  const Agents agents_init(graph_full, 10);
  Agents agents = agents_init;
  const size_t generation_size = 3;
  Generation generation(
      generation_size, graph_full.GetEjectCheckpoints().size(), kept_checkpoint_ratio);

  struct Assignment {
    std::vector<std::vector<Point>> paths;
    double throughput;
    std::vector<size_t> eject_checkpoints_indices;
    Graph graph;
    Agents agents;
  };
  double total_throughput = 0.0;
  double min_throughput = std::numeric_limits<double>::max();
  std::optional<Assignment> best_assignment;

  const size_t steps = std::atoi(argv[4]);
  for (size_t i = 0; i < steps; ++i) {
    for (auto& chromosome : generation.GetChromosomesMutable()) {
      Graph graph = graph_full;
      graph.KeepOnlySelectedCheckpoints(chromosome.eject_checkpoints_permutation);
      task_assigner = task_assigner_init;
      agents = agents_init;
      auto paths = PriorityBasedSearch(agents, graph, task_assigner, 30);
      const double throughput = CalculateThroughput(paths, assignments_cnt);
      chromosome.SetScore(throughput);
      if (!best_assignment || best_assignment->throughput < throughput) {
        if (!best_assignment) {
          best_assignment = Assignment();
        }
        best_assignment->paths = std::move(paths);
        best_assignment->throughput = throughput;
        best_assignment->eject_checkpoints_indices = chromosome.eject_checkpoints_permutation;
        best_assignment->graph = graph;
        best_assignment->agents = agents;
      }
      min_throughput = std::min(min_throughput, throughput);
      total_throughput += throughput;
    }
    generation.Evolve();
  }

  assert(best_assignment);
  for (size_t i = 0; i < best_assignment->paths.size(); ++i) {
    const Agent& cur_agent = best_assignment->agents.At(i);
    std::cout << "Path for agent " << cur_agent.id << " : ";
    for (const auto& position : best_assignment->paths[i]) {
      std::cout << position << " ";
    }
    std::cout << std::endl;
    cur_agent.PrintDebugInfo(std::cout);
  }
  std::cerr << "Worst throughput : " << min_throughput << std::endl;
  std::cerr << "Best throughput : " << best_assignment->throughput << std::endl;
  std::cerr << "Average throughput : "
            << total_throughput / (steps * generation_size) << std::endl;

  return {};
}

int main(int argc, char** argv) {
  GenerateLayout(argc, argv);
}
