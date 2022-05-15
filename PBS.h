#pragma once

#include "agents.h"
#include "graph.h"
#include "task_assigner.h"

#include <future>
#include <vector>

using Paths = std::vector<std::vector<Point>>;

void PriorityBasedSearch(
    Agents agents,
    Graph graph,
    TaskAssigner task_assigner,
    const size_t window_size,
    std::promise<std::pair<Paths, Agents>>&& promise);
