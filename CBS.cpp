#include "CBS.h"

#include "AStar.h"

#include <numeric>

struct Conflict {
  size_t agent_1;
  size_t agent_2;
  size_t ts;
};

namespace {

  size_t CalculateCost(const std::vector<std::vector<Point>>& paths) {
    return std::accumulate(paths.begin(), paths.end(), 0,
        [](size_t cost, const std::vector<Point>& path) {
            return cost + path.size();
    });
  }

  std::optional<Conflict> FindFirstConflict(
      const std::vector<std::vector<Point>>& paths,
      const std::optional<size_t>& window_size) {
    size_t max_timestamp = std::max_element(paths.begin(), paths.end(), []
        (const std::vector<Point>& v1, const std::vector<Point>& v2) {
            return v1.size() < v2.size();
    })->size();
    if (window_size) {
      max_timestamp = std::min(max_timestamp, window_size.value());
    }
    std::map<Point, size_t> position_to_agent;
    for (size_t ts = 0; ts < max_timestamp; ++ts) {
      position_to_agent.clear();
      for (size_t agent_id = 0; agent_id < paths.size(); ++agent_id) {
        if (paths[agent_id].size() <= ts) {
          continue;
        }
        const auto agent_pos = paths[agent_id][ts];
        if (position_to_agent.count(agent_pos)) {
          // Conflict found
          std::cerr << "has conflict for : " << agent_id << " and " << position_to_agent.at(agent_pos) << " at " << ts << std::endl;
          return Conflict{position_to_agent.at(agent_pos), agent_id, ts};
        }
        position_to_agent[agent_pos] = agent_id;
      }
    }
    return std::nullopt;
  }

}

std::vector<std::vector<Point>> GetPaths(
    const Agents& agents,
    const std::unordered_map<size_t,
    std::unordered_map<size_t, std::set<Point>>>& conflicts,
    const Graph& graph) {
  std::vector<std::vector<Point>> result;
  result.reserve(agents.GetSize());
  for (const auto& agent : agents.GetAgents()) {
    auto agent_path = AStar(
        agent,
        conflicts.count(agent.id)
            ? conflicts.at(agent.id)
            : std::unordered_map<size_t, std::set<Point>>{},
        graph);
    result.push_back(std::move(agent_path));
  }
  return result;
}

struct CBSState {
  // agent -> time -> positions
  std::unordered_map<size_t, std::unordered_map<size_t, std::set<Point>>> conflicts;
  std::vector<std::vector<Point>> paths;
  int cost;
};

std::vector<std::vector<Point>> MakeCBSIteration(
    const Agents& agents,
    const Graph& graph,
    const TaskAssigner& task_assigner,
    const size_t window_size) {
  auto states_cmp = [](const CBSState& s1, const CBSState& s2) { return s1.cost < s2.cost; };
  std::multiset<CBSState, decltype(states_cmp)> states(states_cmp);

  CBSState root;
  root.paths = GetPaths(agents, root.conflicts, graph);
  root.cost = CalculateCost(root.paths);
  states.insert(root);

  auto add_state_with_conflict = [&states, &agents, &graph] (
      CBSState state,
      const size_t agent_id,
      const size_t ts,
      const Point& position) {
    state.conflicts[agent_id][ts].insert(position);
    state.paths[agent_id] = AStar(
        agents.GetAgents()[agent_id],
        state.conflicts.at(agent_id),
        graph);
    if (state.paths[agent_id].empty()) {
      return;
    }
    state.cost = CalculateCost(state.paths);
    states.insert(state);
  };

  while (!states.empty()) {
    const CBSState cur_state = *(states.begin());
    states.erase(states.begin());
    const auto conflict = FindFirstConflict(cur_state.paths, window_size);
    if (!conflict) {
      // CBS done
      return cur_state.paths;
    }
    const size_t ts = conflict->ts;
    const Point position = cur_state.paths[conflict->agent_1][ts];

    /*
    std::cerr << "Conflict found for agents "
              << conflict->agent_1 << " and " << conflict->agent_2
              << " at {" << position.first << ", " << position.second << "} on ts "
              << ts << std::endl;
    */

    add_state_with_conflict(cur_state, conflict->agent_1, ts, position);
    add_state_with_conflict(cur_state, conflict->agent_2, ts, position);
  }
  std::cerr << "Something went wrong CBS has no states!" << std::endl;
  return {};
}

std::vector<std::vector<Point>> ConflictBasedSearch(
    Agents& agents, const Graph& graph, TaskAssigner& task_assigner, const size_t window_size) {
  std::vector<std::vector<Point>> result(agents.GetSize());
  do {
    agents.UpdateTasksLists(task_assigner, window_size);
    const auto paths_prefixes = MakeCBSIteration(agents, graph, task_assigner, window_size);
    /*
    std::cerr << "ok, got paths" << std::endl;
    for (const auto& agent : paths_prefixes) {
      for (const auto& point : agent) {
        std::cerr << point << " ";
      }
      std::cerr << std::endl;
    }
    */
    agents.DeleteCompletedTasks(paths_prefixes, window_size);
    std::cerr << "remaining tasks : " << task_assigner.RemainingTasks() << std::endl;
    for (size_t i = 0; i < paths_prefixes.size(); ++i) {
      for (size_t j = 0; j < std::min(window_size, paths_prefixes[i].size()); ++j) {
        result[i].push_back(paths_prefixes[i][j]);
      }
    }
  } while (task_assigner.HasAssignments());
  return result;
}
