#pragma once

#include <utility>
#include <queue>

#include "common.h"
#include "graph.h"
#include "yaml-cpp/yaml.h"

struct Agent {
  Point start;
  std::queue<Point> locations_to_visit;
  size_t id;
};

class Agents {
public:
  Agents(const YAML::Node& yaml_agents);
  Agents(const Graph& graph, const size_t agents_num, const size_t seed = 42);

  const std::vector<Agent>& GetAgents() const;
  const size_t GetSize() const;

private:
  std::vector<Agent> agents;
};
