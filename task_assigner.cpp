#include "task_assigner.h"

#include <cassert>

TaskAssigner::TaskAssigner(const Graph& graph, const size_t assignments_cnt, const size_t seed) {
  srand(seed);
  const auto& induct_checkpoints = graph.GetInductCheckpoints();
  const auto& eject_checkpoints = graph.GetEjectCheckpoints();
  assert(!induct_checkpoints.empty() && "Need at least one induct checkpoint!");
  assert(!eject_checkpoints.empty() && "Need at least one eject checkpoint!");
  std::cout << "Generating initial " << assignments_cnt << " checkpoints: ";
  for (size_t i = 0; i < assignments_cnt; ++i) {
    const size_t start_idx = rand() % induct_checkpoints.size();
    const size_t finish_idx = rand() % eject_checkpoints.size();
    Assignment cur_assignment(induct_checkpoints[start_idx], eject_checkpoints[finish_idx]);
    std::cout << cur_assignment << " ";
    assignments.push_back(cur_assignment);
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
