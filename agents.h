#pragma once

#include <utility>

#include "yaml-cpp/yaml.h"

struct Agent {
  std::pair<int, int> start;
  std::pair<int, int> finish;
  size_t id;
};

class Agents {
public:
  Agents(const YAML::Node& yaml_agents);

  const std::vector<Agent>& GetAgents() const;
  const size_t GetSize() const;

private:
  std::vector<Agent> agents;
};
