#pragma once

#include "agents.h"
#include "graph.h"
#include "task_assigner.h"

#include <vector>
#include <unordered_map>

std::vector<std::vector<Point>> GetPaths(
    const Agents& agents,
    const std::unordered_map<size_t,
    std::unordered_map<size_t, std::set<Point>>>& conflicts,
    const Graph& graph);

std::vector<std::vector<Point>> MakeCBSIteration(
    const Agents& agents,
    const Graph& graph,
    const TaskAssigner& task_assigner,
    const size_t window_size);

std::vector<std::vector<Point>> ConflictBasedSearch(
    Agents& agents, const Graph& graph, TaskAssigner& task_assigner, const size_t window_size);
