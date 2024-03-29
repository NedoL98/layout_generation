#include "topsort.h"

#include <algorithm>

namespace {

void DFS(
    const std::vector<std::vector<size_t>>& priority_graph,
    std::vector<size_t>& result,
    std::vector<bool>& used,
    const size_t v) {
  used[v] = true;
  for (size_t u : priority_graph[v]) {
    if (!used[u]) {
      DFS(priority_graph, result, used, u);
    }
  }
  result.push_back(v);
}

}

std::optional<std::vector<size_t>> TopSort(const std::vector<std::vector<size_t>>& priority_graph) {
  std::vector<bool> used(priority_graph.size(), false);
  std::vector<size_t> result;
  result.reserve(priority_graph.size());
  for (size_t i = 0; i < used.size(); ++i) {
    if (!used[i]) {
      DFS(priority_graph, result, used, i);
    }
  }
  std::reverse(result.begin(), result.end());

  std::vector<size_t> vertex_to_pos(result.size());
  for (size_t i = 0; i < result.size(); ++i) {
    vertex_to_pos[result[i]] = i;
  }
  for (size_t v = 0; v < priority_graph.size(); ++v) {
    for (const auto u : priority_graph[v]) {
      if (vertex_to_pos[v] > vertex_to_pos[u]) {
        return std::nullopt;
      }
    }
  }

  return result;
}