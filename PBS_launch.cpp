#include "agents.h"
// #include "CBS.h"
#include "PBS.h"
#include "graph.h"
#include "task_assigner.h"

#include "yaml-cpp/yaml.h"

#include <iostream>
#include <optional>
#include <set>
#include <unordered_map>

int main(int argc, char** argv) {
  if (argc != 4) {
    std::cerr << "please specify following params: " << std::endl;
    std::cerr << "    - path to data file" << std::endl;
    std::cerr << "    - number of assignments" << std::endl;
    std::cerr << "    - deleted eject checkpoint ratio" << std::endl;
    exit(0);
  }
  /*
  YAML::Node yaml_config = YAML::LoadFile(argv[1]);
  ASSERT(yaml_config["map"] && "No map found in config file");
  Graph graph(yaml_config["map"]);
  ASSERT(yaml_config["agents"] && "No agents found in config file");
  Agents agents(yaml_config["agents"]);
  */
  Graph graph(argv[1], std::stod(argv[3]));
  const size_t assignments_cnt = std::atoi(argv[2]);
  TaskAssigner task_assigner(graph, assignments_cnt);
  // Set chosen induct checkpoints as obstacles
  graph.SetInductCheckpointsAsObstacles(task_assigner.GetAllRemainingAssigments());
  Agents agents(graph, 10);
  const auto paths = PriorityBasedSearch(agents, graph, task_assigner, 30);
  for (size_t i = 0; i < paths.size(); ++i) {
    const Agent& cur_agent = agents.At(i);
    std::cout << "Path for agent " << cur_agent.id << " : ";
    for (const auto& position : paths[i]) {
      std::cout << position << " ";
    }
    std::cout << std::endl;
    cur_agent.PrintDebugInfo(std::cout);
  }
  const double throughput = CalculateThroughput(paths, assignments_cnt);
  std::cerr << "Throughtput: " << throughput << std::endl;
}
