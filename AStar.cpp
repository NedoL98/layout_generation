#include "AStar.h"

struct AStarState {
  std::vector<Point> path;
  size_t label;
  size_t ts;

  size_t LowerBoundToGoal(const Point& goal) const {
    assert(!path.empty() && "error, path is empty!");
    return std::abs(path.back().x - goal.x) + std::abs(path.back().y - goal.y);
  }
};

std::vector<Point> AStar(
    const Agent& agent,
    const std::unordered_map<size_t, std::set<Point>>& agent_conflicts,
    const Graph& graph) {
  if (agent.locations_to_visit.empty()) {
    return {};
  }
  auto states_cmp = [&agent](const AStarState& s1, const AStarState& s2) {
    if (s1.label < s2.label) {
      return false;
    } else if (s1.label > s2.label) {
      return true;
    } else {
      return s1.ts + s1.LowerBoundToGoal(agent.locations_to_visit[s1.label])
          < s2.ts + s2.LowerBoundToGoal(agent.locations_to_visit[s2.label]);
    }
  };
  std::multiset<AStarState, decltype(states_cmp)> states(states_cmp);
  // {position, ts}
  std::set<std::pair<Point, size_t>> used;

  size_t start_ts = 0;
  while (states.empty()) {
    if (!agent_conflicts.count(start_ts) || !agent_conflicts.at(start_ts).count(agent.start)) {
      states.insert({{agent.start}, 0, start_ts});
      used.insert({agent.start, start_ts});
    }
    ++start_ts;
  }
  while (!states.empty()) {
    const AStarState cur_state = *(states.begin());

    states.erase(states.begin());
    const auto neighbours = graph.GetNeighbours(cur_state.path.back());
    const size_t ts = cur_state.ts;

    for (const auto& neighbour : neighbours) {
      if ((!agent_conflicts.count(ts + 1) || !agent_conflicts.at(ts + 1).count(neighbour))
          && !used.count({neighbour, ts + 1})) {
        auto new_path = cur_state.path;
        new_path.push_back(neighbour);

        AStarState new_state = cur_state;
        new_state.path = new_path;
        if (new_state.path.back() == agent.locations_to_visit[new_state.label]) {
          ++new_state.label;
        }
        ++new_state.ts;

        if (new_state.label == agent.locations_to_visit.size()) {
          // AStar done
          return new_state.path;
        }

        states.insert(std::move(new_state));
        used.insert({neighbour, new_state.ts});
      }
    }
  }
  std::cerr << "AStar is stuck on " << agent.id << "!" << std::endl;
  return {};
}
