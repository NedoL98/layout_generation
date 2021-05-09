#pragma once

#include "agents.h"
#include "graph.h"

#include <cassert>
#include <vector>
#include <unordered_map>

std::vector<Point> AStar(
    const Agent& agent,
    const std::unordered_map<size_t, std::set<Point>>& agent_conflicts,
    const Graph& graph);