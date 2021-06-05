#include "task_assigner.h"

#include <cassert>

TaskAssigner::TaskAssigner(const Graph& graph, const size_t assignments_cnt, const size_t seed) {
  srand(seed);
  const auto& checkpoints = graph.GetCheckpoints();
  assert(checkpoints.size() >= 2 && "need at least two checkpoints!");
  for (size_t i = 0; i < assignments_cnt; ++i) {
    const size_t start_idx = rand() % checkpoints.size();
    size_t finish_idx = rand() % checkpoints.size();
    while (finish_idx == start_idx) {
      finish_idx = rand() % checkpoints.size();
    }
    assignments.push_back({checkpoints[start_idx], checkpoints[finish_idx]});
  }
}

std::optional<Assignment> TaskAssigner::GetNextAssignment() {
  if (assignments.empty()) {
    return std::nullopt;
  }
  const auto front_assignment = assignments.front();
  assignments.pop_front();
  return front_assignment;
}

bool TaskAssigner::HasAssignments() const {
  return !assignments.empty();
}

size_t TaskAssigner::RemainingTasks() const {
  return assignments.size();
}
