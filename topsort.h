#pragma once

#include <optional>
#include <vector>

std::optional<std::vector<size_t>> TopSort(const std::vector<std::vector<size_t>>& priority_graph);
