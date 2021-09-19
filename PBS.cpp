#include "PBS.h"

#include "AStar.h"
#include "topsort.h"

#include <optional>
#include <unordered_map>

struct PBSState {
  // agent -> time -> positions
  std::unordered_map<size_t, std::unordered_map<size_t, std::set<Point>>> vertex_conflicts;
  // agent -> time -> edge
  std::unordered_map<size_t, std::unordered_map<size_t, std::set<Edge>>> edge_conflicts;
  std::vector<std::vector<Point>> paths;
  std::vector<std::vector<size_t>> priority_graph;
  int cost;

  PBSState(const size_t size)
  : paths(size)
  , priority_graph(size) {
    for (size_t i = 0; i < size; ++i) {
      vertex_conflicts[i] = {};
      edge_conflicts[i] = {};
    }
  }
};

namespace {

bool HasConflict(const std::vector<Point>& lhs, const std::vector<Point>& rhs) {
  for (size_t ts = 0; ts < std::min(lhs.size(), rhs.size()); ++ts) {
    // todo : add more constraints
    if (lhs[ts] == rhs[ts]) {
      return true;
    }
  }
  return false;
}

void UpdatePaths(
    const Agents& agents,
    const Graph& graph,
    PBSState& pbs_state,
    const std::optional<size_t> update_path_for) {
  const std::vector<size_t> topsort_order = TopSort(pbs_state.priority_graph);

  std::vector<bool> path_updated(agents.GetSize(), false);

  for (size_t i = 0; i < topsort_order.size(); ++i) {
    const size_t agent_id = topsort_order[i];
    const Agent& agent = agents.At(agent_id);
    assert(agent_id == agent.id);

    // todo : update AStar according to paper

    if (!update_path_for) {
      // Update all paths
      pbs_state.paths[agent_id] = AStar(
        agent,
        pbs_state.vertex_conflicts.at(agent_id),
        pbs_state.edge_conflicts.at(agent_id),
        graph,
        std::cref(pbs_state.paths),
        std::cref(topsort_order),
        i);
        path_updated[agent_id] = true;
    } else {
      // Update path only for the chosen agent and for all conflicting agents with lower priority
      assert(update_path_for.value() < agents.GetSize());
      bool update_path = (update_path_for.value() == agent_id);
      if (!update_path) {
        for (size_t j = 0; j < i; ++j) {
          const size_t higher_priority_agent_id = topsort_order[j];
          if (HasConflict(pbs_state.paths[agent_id], pbs_state.paths[higher_priority_agent_id])) {
            update_path = true;
            break;
          }
        }
      }
      if (update_path) {
        pbs_state.paths[agent_id] = AStar(
          agent,
          pbs_state.vertex_conflicts.at(agent.id),
          pbs_state.edge_conflicts.at(agent.id),
          graph,
          std::cref(pbs_state.paths),
          std::cref(topsort_order),
          i);
        path_updated[agent_id] = true;
      }
    }
  }
}

std::vector<std::vector<Point>> MakePBSIteration(
    const Agents& agents,
    const Graph& graph,
    const TaskAssigner& task_assigner,
    const size_t window_size) {
  auto states_cmp = [](const PBSState& s1, const PBSState& s2) { return s1.cost < s2.cost; };
  std::multiset<PBSState, decltype(states_cmp)> states(states_cmp);

  PBSState root(agents.GetSize());
  UpdatePaths(agents, graph, root, std::nullopt);
  root.cost = CalculateCost(root.paths);
  states.insert(root);

  auto add_state_with_conflict = [&states, &agents, &graph] (
      PBSState state,
      const size_t agent_id_low_priority,
      const size_t agent_id_high_priority,
      const ConflictBase& conflict) {
    const size_t ts = conflict.ts;

    if (conflict.conflict_type == ConflictType::VertexConflict) {
      const Point position = dynamic_cast<const VertexConflict&>(conflict).conflicting_vertex;
      assert(position == state.paths[agent_id_low_priority][ts]);
      state.vertex_conflicts[agent_id_low_priority][ts].insert(position);
    } else if (conflict.conflict_type == ConflictType::EdgeConflict) {
      Edge edge = dynamic_cast<const EdgeConflict&>(conflict).conflicting_edge;
      if (state.paths[agent_id_low_priority][ts - 1] != edge.first
          && state.paths[agent_id_low_priority][ts] != edge.second) {
        std::swap(edge.second, edge.first);
      }
      assert(edge.first == state.paths[agent_id_low_priority][ts - 1]);
      assert(edge.second == state.paths[agent_id_low_priority][ts]);
      state.edge_conflicts[agent_id_low_priority][ts].insert(std::move(edge));
    } else {
      std::cerr << "Conflict has no type!" << std::endl;
      exit(0);
    }

    state.priority_graph[agent_id_high_priority].push_back(agent_id_low_priority);
    UpdatePaths(agents, graph, state, agent_id_low_priority);

    if (state.paths[agent_id_low_priority].empty()) {
      return;
    }
    state.cost = CalculateCost(state.paths);
    states.insert(std::move(state));
  };

  while (!states.empty()) {
    const PBSState cur_state = *(states.begin());
    states.erase(states.begin());
    auto conflict = FindFirstConflict(cur_state.paths, window_size);
    if (!conflict) {
      // CBS done
      return cur_state.paths;
    }
    add_state_with_conflict(cur_state, conflict->agent_1, conflict->agent_2, *conflict);
    add_state_with_conflict(cur_state, conflict->agent_2, conflict->agent_1, *conflict);
  }
  std::cerr << "Something went wrong CBS has no states!" << std::endl;
  return {};
}

}

// todo : this is the same as CBS, merge them
std::vector<std::vector<Point>> PriorityBasedSearch(
    Agents& agents, const Graph& graph, TaskAssigner& task_assigner, const size_t window_size) {
  std::vector<std::vector<Point>> result(agents.GetSize());
  bool has_tasks = false;
  do {
    agents.UpdateTasksLists(task_assigner, window_size, graph);
    // todo : fix bug with not waiting after new window size
    const auto paths_prefixes = MakePBSIteration(agents, graph, task_assigner, window_size);
    has_tasks = agents.DeleteCompletedTasks(paths_prefixes, window_size);
    std::cerr << "remaining tasks : " << task_assigner.RemainingTasks() << std::endl;
    for (size_t i = 0; i < paths_prefixes.size(); ++i) {
      for (size_t j = 0; j < std::min(window_size, paths_prefixes[i].size()); ++j) {
        result[i].push_back(paths_prefixes[i][j]);
      }
    }
  } while (task_assigner.HasAssignments() || has_tasks);
  return result;
}