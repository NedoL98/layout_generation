#include "agents.h"

Agents::Agents(const YAML::Node& yaml_agents) {
  agents.clear();
  size_t agent_id = 0;
  for (const auto& yaml_agent : yaml_agents) {
    agents.push_back({
      yaml_agent["start"].as<std::pair<int, int>>(),
      yaml_agent["goal"].as<std::pair<int, int>>(),
      agent_id});
    ++agent_id;
  }
}

const std::vector<Agent>& Agents::GetAgents() const {
  return agents;
}

const size_t Agents::GetSize() const {
  return agents.size();
}
