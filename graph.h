#pragma once

#include <set>
#include <utility>

#include "yaml-cpp/yaml.h"

class Graph{
public:
  Graph(const YAML::Node& yaml_graph);

  std::vector<std::pair<int, int>> GetNeighbours(const std::pair<int, int>& pos) const;

private:
  int width;
  int height;
  std::set<std::pair<int, int>> obstacles;
};
