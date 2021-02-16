#include "graph.h"

#include <cassert>
#include <iostream>

Graph::Graph(const YAML::Node& yaml_graph) {
  width = yaml_graph["dimensions"].as<std::pair<int, int>>().first;
  height = yaml_graph["dimensions"].as<std::pair<int, int>>().second;
  for (const auto& obstacle : yaml_graph["obstacles"]) {
    obstacles.insert(obstacle.as<std::pair<int, int>>());
  }
}

std::vector<std::pair<int, int>> Graph::GetNeighbours(const std::pair<int, int>& pos) const {
  std::vector<std::pair<int, int>> neighbours;
  for (const int dx : {-1, 0, 1}) {
    for (const int dy : {-1, 0, 1}) {
      if (abs(dx) + abs(dy) <= 1
          && pos.first + dx < width && pos.first + dx >= 0
          && pos.second + dy < height && pos.second + dy >= 0
          && !obstacles.count({pos.first + dx, pos.second + dy})) {
        neighbours.push_back({pos.first + dx, pos.second + dy});
      }
    }
  }
  return neighbours;
}
