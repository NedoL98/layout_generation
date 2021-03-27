#include "agents.h"
#include "graph.h"
#include "task_assigner.h"

#include "yaml-cpp/yaml.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <numeric>
#include <set>
#include <unordered_map>

struct AStarState {
  std::vector<Point> path;
  size_t ts;
  // todo : add admissable heuristic
};

std::vector<Point> AStar(
    const Agent& agent,
    const std::unordered_map<size_t, std::set<Point>>& agent_conflicts,
    const Graph& graph) {
  auto states_cmp = [](const AStarState& s1, const AStarState& s2) {
    return s1.ts < s2.ts;
  };
  std::multiset<AStarState, decltype(states_cmp)> states(states_cmp);
  // {position, ts}
  std::set<std::pair<Point, size_t>> used;

  size_t start_ts = 0;
  while (states.empty()) {
    if (!agent_conflicts.count(start_ts) || !agent_conflicts.at(start_ts).count(agent.start)) {
      states.insert({{agent.start}, start_ts});
      used.insert({agent.start, start_ts});
    }
    ++start_ts;
  }
  while (!states.empty()) {
    const AStarState cur_state = *(states.begin());
    states.erase(states.begin());
    /*
    if (cur_state.path.back() == agent.finish) {
      // AStar done
      return cur_state.path;
    }
    */
    const auto neighbours = graph.GetNeighbours(cur_state.path.back());
    const size_t ts = cur_state.ts;

    for (const auto& neighbour : neighbours) {
      if ((!agent_conflicts.count(ts + 1) || !agent_conflicts.at(ts + 1).count(neighbour))
          && !used.count({neighbour, ts + 1})) {
        auto new_path = cur_state.path;
        new_path.push_back(neighbour);
        states.insert({new_path, ts + 1});
        used.insert({neighbour, ts + 1});
      }
    }
  }
  std::cerr << "AStar is stuck on " << agent.id << "!" << std::endl;
  return {};
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

size_t CalculateCost(const std::vector<std::vector<Point>>& paths) {
  return std::accumulate(paths.begin(), paths.end(), 0,
      [](size_t cost, const std::vector<Point>& path) {
          return cost + path.size();
  });
}

struct Conflict {
  size_t agent_1;
  size_t agent_2;
  size_t ts;
};

std::optional<Conflict> FindFirstConflict(
    const std::vector<std::vector<Point>>& paths) {
    size_t max_timestamp = std::max_element(paths.begin(), paths.end(), []
      (const std::vector<Point>& v1, const std::vector<Point>& v2) {
          return v1.size() < v2.size();
  })->size();
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
        return Conflict{position_to_agent.at(agent_pos), agent_id, ts};
      }
      position_to_agent[agent_pos] = agent_id;
    }
  }
  return std::nullopt;
}

struct CBSState {
  // agent -> time -> positions
  std::unordered_map<size_t, std::unordered_map<size_t, std::set<Point>>> conflicts;
  std::vector<std::vector<Point>> paths;
  int cost;
};

std::vector<std::vector<Point>> ConflictBasedSearch(
    const Agents& agents,
    const Graph& graph,
    TaskAssigner& task_assigner,
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
    const auto conflict = FindFirstConflict(cur_state.paths);
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
  const auto paths = ConflictBasedSearch(agents, graph, task_assigner, 10);
  for (size_t i = 0; i < paths.size(); ++i) {
    std::cerr << "Path for agent " << i << " : ";
    for (const auto& position : paths[i]) {
      std::cerr << position << " ";
    }
    std::cerr << std::endl;
  }
}
