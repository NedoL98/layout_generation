#include "agents.h"

#include <cassert>

void Agent::PrintDebugInfo(std::ostream& ostream) const {
  ostream << "All assignments for agent " << id << ": ";
  for (const auto& assignment : all_assignments) {
    ostream << assignment << " ";
  }
  ostream << std::endl;
}

size_t Agent::CalculateLowerBound() const {
  size_t result = 0;
  for (size_t i = 0; i < locations_to_visit.size(); ++i) {
    const Point& prev_location = (i == 0) ? start : locations_to_visit[i - 1];
    const Point& cur_location = locations_to_visit[i];
    result += std::abs(prev_location.x - cur_location.x)
        + std::abs(prev_location.y - cur_location.y);
  }
  return result;
}

Agents::Agents(const YAML::Node& yaml_agents) {
  agents.clear();
  size_t agent_id = 0;
  for (const auto& yaml_agent : yaml_agents) {
    agents.push_back(Agent(yaml_agent["start"].as<std::pair<int, int>>(), agent_id));
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
    agents.push_back(Agent(spare_locations[rand_idx], agent_id));
    // redo this
    spare_locations.erase(spare_locations.begin() + rand_idx);
    ++agent_id;
  }
}

const std::vector<Agent>& Agents::GetAgents() const {
  return agents;
}

const Agent& Agents::At(const size_t index) const {
  return agents.at(index);
}

const size_t Agents::GetSize() const {
  return agents.size();
}

void Agents::UpdateTasksLists(TaskAssigner& task_assigner, const size_t window_size) {
  std::cerr << "updating tasks list : " << std::endl;
  for (auto& agent : agents) {
    // todo: optimize this
    while (agent.CalculateLowerBound() < window_size) {
      const auto next_task_opt = task_assigner.GetNextAssignment();
      if (next_task_opt) {
        agent.locations_to_visit.push_back(next_task_opt->start);
        agent.locations_to_visit.push_back(next_task_opt->finish);
        agent.all_assignments.push_back(next_task_opt.value());
      } else {
        break;
      }
    }
    for (const auto& point : agent.locations_to_visit) {
      std::cerr << point << " ";
    }
    std::cerr << std::endl;
  }
  std::cerr << "~~~~~~~" << std::endl;
}

bool Agents::DeleteCompletedTasks(
    const std::vector<std::vector<Point>>& path_prefixes, const size_t window_size) {
  std::cerr << "deleting completed tasks : " << std::endl;
  bool has_tasks = false;
  for (size_t i = 0; i < path_prefixes.size(); ++i) {
    for (size_t j = 0; j < std::min(window_size, path_prefixes[i].size()); ++j) {
      const auto& cur_point = path_prefixes[i][j];
      if (!agents[i].locations_to_visit.empty()
          && agents[i].locations_to_visit.front() == cur_point) {
        agents[i].locations_to_visit.pop_front();
      }
      has_tasks |= !agents[i].locations_to_visit.empty();
      if (j + 1 == std::min(window_size, path_prefixes[i].size())) {
        std::cerr << "old position : " << agents[i].start << std::endl;
        agents[i].start = cur_point;
      }
    }

    std::cerr << "new position : " << agents[i].start << std::endl;
    for (const auto& point : agents[i].locations_to_visit) {
      std::cerr << point << " ";
    }
    std::cerr << std::endl;
  }
  std::cerr << "~~~~~~~~~" << std::endl;
  return has_tasks;
}
