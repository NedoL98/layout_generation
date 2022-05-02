#include "AStar.h"

#include <unordered_set>

struct AStarState {
  std::vector<Point> path;
  size_t label;
  size_t ts;
  std::optional<size_t> waiting_duration_opt;

  size_t LowerBoundToGoal(const Point& goal) const {
    ASSERT(!path.empty() && "error, path is empty!");
    return std::abs(path.back().x - goal.x) + std::abs(path.back().y - goal.y);
  }
};

struct AStarUsedState {
  Point point;
  size_t ts;
  std::optional<size_t> waiting_duration_opt;
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

  auto used_states_cmp = [](const AStarUsedState& s1, const AStarUsedState& s2) {
    if (s1.ts < s2.ts) {
      return false;
    } else if (s1.ts > s2.ts) {
      return true;
    } else {
      if (s1.point < s2.point) {
        return false;
      } else if (s1.point > s2.point) {
        return true;
      } else {
        return s1.waiting_duration_opt < s2.waiting_duration_opt;
      }
    }
  };

  std::multiset<AStarState, decltype(states_cmp)> states(states_cmp);
  std::set<AStarUsedState, decltype(used_states_cmp)> used(used_states_cmp);

  size_t start_ts = 0;
  while (states.empty()) {
    if (!vertex_conflicts.count(start_ts) || !vertex_conflicts.at(start_ts).count(agent.start)) {
      states.insert({{agent.start}, 0, start_ts, agent.waiting_duration_opt});
      used.insert({agent.start, 0, agent.waiting_duration_opt});
    }
    ++start_ts;
  }

  const auto do_visit = [&] (
      const Point& position,
      const Point& next_position,
      const size_t ts,
      const std::optional<size_t>& waiting_duration_opt) {
    if (position != next_position && waiting_duration_opt) {
      // Need to wait at checkpoint
      return false;
    }
    if (used.count({next_position, ts, waiting_duration_opt})) {
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
          if (paths_opt->get()[agent_idx][ts] == position
              && paths_opt->get()[agent_idx][ts - 1] == next_position) {
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

    // todo : fix this
    if (ts >= 10 * 1000) {
      std::cerr << "AStar is stuck on " << agent.id << "!" << std::endl;
      return {};
    }

    for (const auto& neighbour : neighbours) {
      if (!do_visit(cur_state.path.back(), neighbour, ts + 1, cur_state.waiting_duration_opt)) {
        continue;
      }
      auto new_path = cur_state.path;
      new_path.push_back(neighbour);

      AStarState new_state = cur_state;
      new_state.path = new_path;
      if (new_state.waiting_duration_opt) {
        if (new_state.waiting_duration_opt.value() + 1 >= graph.GetTimeToWaitNearCheckpoints()) {
          new_state.waiting_duration_opt = std::nullopt;
        } else {
          ++new_state.waiting_duration_opt.value();
        }
      } else if (new_state.path.back() == agent.locations_to_visit[new_state.label]) {
        ++new_state.label;
        if (graph.GetTimeToWaitNearCheckpoints() > 1) {
          new_state.waiting_duration_opt = 1;
        }
      }
      ++new_state.ts;

      if (new_state.label == agent.locations_to_visit.size()
          && (!new_state.waiting_duration_opt || graph.GetTimeToWaitNearCheckpoints() <= 1)) {
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
