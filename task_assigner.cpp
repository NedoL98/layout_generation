#include "task_assigner.h"

#include <cassert>

TaskAssigner::TaskAssigner(const Graph& graph, const size_t assignments_cnt, const size_t seed) {
  srand(seed);
  const size_t induct_checkpoints_size = graph.GetInductCheckpoints().size();
  const size_t eject_checkpoints_size = graph.GetEjectCheckpoints().size();
  assert(induct_checkpoints_size > 0 && "Need at least one induct checkpoint!");
  assert(eject_checkpoints_size > 0 && "Need at least one eject checkpoint!");
  if (assignments_cnt < std::max(induct_checkpoints_size, eject_checkpoints_size)) {
    std::cerr
        << "Consider increasing number of assignments, not all checkpoints are visited"
        << std::endl;
  }
  std::cerr << "Generating initial " << assignments_cnt << " checkpoints: ";
  const auto fill_and_shuffle = [] (size_t size) {
    std::vector<size_t> permutation(size);
    std::iota(permutation.begin(), permutation.end(), 0);
    std::random_shuffle(permutation.begin(), permutation.end());
    return permutation;
  };
  const auto induct_permutation = fill_and_shuffle(induct_checkpoints_size);
  const auto eject_permutation = fill_and_shuffle(eject_checkpoints_size);

  // This assures that every checkpoint is used at least once
  size_t idx = 0;
  while (assignments.size() < assignments_cnt) {
    const size_t start_idx =
        idx < induct_checkpoints_size
        ? induct_permutation[idx]
        : induct_permutation[rand() % induct_checkpoints_size];
    const size_t finish_idx =
        idx < eject_checkpoints_size
        ? eject_permutation[idx]
        : eject_permutation[rand() % eject_checkpoints_size];
    Assignment cur_assignment(start_idx, finish_idx);
    assignments.push_back(cur_assignment);
    ++idx;
  }
  std::random_shuffle(assignments.begin(), assignments.end());
}

std::vector<Assignment> TaskAssigner::GetAllRemainingAssigments() const {
  return std::vector<Assignment>(assignments.begin(), assignments.end());
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
