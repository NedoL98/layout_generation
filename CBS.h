#pragma once

#include "agents.h"
#include "graph.h"
#include "task_assigner.h"

#include <vector>
#include <unordered_map>

std::vector<std::vector<Point>> ConflictBasedSearch(
    Agents& agents, const Graph& graph, TaskAssigner& task_assigner, const size_t window_size);
