#pragma once

#include "agents.h"
#include "graph.h"

#include <cassert>
#include <vector>
#include <unordered_map>

std::vector<Point> AStar(
    const Agent& agent,
    const std::unordered_map<size_t, std::set<Point>>& vertex_conflicts,
    const std::unordered_map<size_t, std::set<Edge>>& edge_conflicts,
    const Graph& graph,
    const std::optional<std::reference_wrapper<const std::vector<std::vector<Point>>>> paths_opt = std::nullopt,
    const std::optional<std::reference_wrapper<const std::vector<size_t>>> topsort_order_opt = std::nullopt,
    const std::optional<size_t> agent_topsort_idx = std::nullopt);
