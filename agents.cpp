#include "agents.h"

#include <cassert>

Agents::Agents(const YAML::Node& yaml_agents) {
  agents.clear();
  size_t agent_id = 0;
  for (const auto& yaml_agent : yaml_agents) {
    agents.push_back({
      yaml_agent["start"].as<std::pair<int, int>>(),
      // yaml_agent["goal"].as<std::pair<int, int>>(),
      {},
      agent_id});
    ++agent_id;
  }
}

Agents::Agents(const Graph& graph, const size_t agents_num, const size_t seed) {
  agents.reserve(agents_num);
  auto spare_locations = graph.GetSpareLocations();
  assert(spare_locations.size() >= agents_num && "can't place all the agents!");
  size_t agent_id = 0;
  for (size_t i = 0; i < agents_num; ++i) {
    size_t rand_idx = rand() % spare_locations.size();
    agents.push_back({
      spare_locations[rand_idx],
      {},
      agent_id});
    // redo this
    spare_locations.erase(spare_locations.begin() + rand_idx);
    ++agent_id;
  }
}

const std::vector<Agent>& Agents::GetAgents() const {
  return agents;
}

const size_t Agents::GetSize() const {
  return agents.size();
}
