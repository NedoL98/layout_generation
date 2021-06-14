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
    const std::unordered_map<size_t, std::set<Point>>& vertex_conflicts,
    const std::unordered_map<size_t, std::set<Edge>>&  edge_conflicts,
    const Graph& graph,
    const std::optional<std::reference_wrapper<const std::vector<std::vector<Point>>>> paths_opt,
    const std::optional<std::reference_wrapper<const std::vector<size_t>>> topsort_order_opt,
    const std::optional<size_t> agent_topsort_idx_opt) {
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
    if (!vertex_conflicts.count(start_ts) || !vertex_conflicts.at(start_ts).count(agent.start)) {
      states.insert({{agent.start}, 0, start_ts});
      used.insert({agent.start, start_ts});
    }
    ++start_ts;
  }

  const auto do_visit = [&] (const Point& position, const Point& next_position, const size_t ts) {
    if (used.count({next_position, ts})) {
      // State was visited earlier
      return false;
    }
    if (vertex_conflicts.count(ts) && vertex_conflicts.at(ts).count(next_position)) {
      // Forbidden by vertex conflict
      return false;
    }
    if (edge_conflicts.count(ts) && edge_conflicts.at(ts).count({position, next_position})) {
      // Forbidden by edge conflict
      return false;
    }
    if (paths_opt && topsort_order_opt && agent_topsort_idx_opt) {
      for (size_t i = 0; i < agent_topsort_idx_opt.value(); ++i) {
        size_t agent_idx = topsort_order_opt->get()[i];
        if (ts >= paths_opt->get()[agent_idx].size()) {
          continue;
        }
        if (paths_opt->get()[agent_idx][ts] == next_position) {
          // Has vertex conflict with higher priority agent
          return false;
        }
        if (ts > 0) {
          if (paths_opt->get()[agent_idx][ts] == next_position
              && paths_opt->get()[agent_idx][ts - 1] == position) {
            // Has edge conflict with higher priority agent
            return false;
          }
        }
      }
    }
    return true;
  };

  while (!states.empty()) {
    const AStarState cur_state = *(states.begin());

    states.erase(states.begin());
    const auto neighbours = graph.GetNeighbours(cur_state.path.back());
    const size_t ts = cur_state.ts;

    for (const auto& neighbour : neighbours) {
      if (!do_visit(cur_state.path.back(), neighbour, ts + 1)) {
        continue;
      }
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
  std::cerr << "AStar is stuck on " << agent.id << "!" << std::endl;
  return {};
}
