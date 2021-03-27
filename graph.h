#pragma once

#include <set>
#include <utility>

#include "common.h"
#include "yaml-cpp/yaml.h"

class Graph{
public:
  Graph(const YAML::Node& yaml_graph);
  Graph(const char* filename);

  const std::vector<Point>& GetCheckpoints() const;
  const std::vector<Point> GetSpareLocations() const;

  std::vector<Point> GetNeighbours(const Point& pos) const;

private:
  int width;
  int height;
  std::set<Point> obstacles;
  std::vector<Point> checkpoints;
};
