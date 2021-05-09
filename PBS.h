#pragma once

#include "agents.h"
#include "graph.h"
#include "task_assigner.h"

#include <vector>

std::vector<std::vector<Point>> PriorityBasedSearch(
    Agents& agents, const Graph& graph, TaskAssigner& task_assigner, const size_t window_size);