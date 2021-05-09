#include "agents.h"
#include "CBS.h"
#include "graph.h"
#include "task_assigner.h"

#include "yaml-cpp/yaml.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <set>
#include <unordered_map>

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "path to data file is not specified" << std::endl;
    return 0;
  }
  /*
  YAML::Node yaml_config = YAML::LoadFile(argv[1]);
  assert(yaml_config["map"] && "No map found in config file");
  Graph graph(yaml_config["map"]);
  assert(yaml_config["agents"] && "No agents found in config file");
  Agents agents(yaml_config["agents"]);
  */
  Graph graph(argv[1]);
  TaskAssigner task_assigner(graph, 100);
  Agents agents(graph, 10);
  const auto paths = ConflictBasedSearch(agents, graph, task_assigner, 30);
  for (size_t i = 0; i < paths.size(); ++i) {
    std::cerr << "Path for agent " << i << " : ";
    for (const auto& position : paths[i]) {
      std::cerr << position << " ";
    }
    std::cerr << std::endl;
  }
}
