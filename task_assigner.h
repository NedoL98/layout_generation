#pragma once

#include "common.h"
#include "graph.h"

#include <deque>
#include <optional>

class TaskAssigner {
public:
  TaskAssigner(const Graph& graph, const size_t assignments_cnt, const size_t seed = 42);

  std::vector<Assignment> GetAllRemainingAssigments() const;
  std::optional<Assignment> GetNextAssignment();
  bool HasAssignments() const;
  size_t RemainingTasks() const;
private:
  std::deque<Assignment> assignments;
};
