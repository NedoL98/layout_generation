#pragma once

#include <set>
#include <utility>

#include "common.h"
#include "yaml-cpp/yaml.h"

class Graph{
public:
  Graph(const YAML::Node& yaml_graph);
  Graph(const char* filename);

  const std::vector<Point>& GetEjectCheckpoints() const;
  const std::vector<Point>& GetInductCheckpoints() const;
  const std::vector<Point> GetSpareLocations() const;

  std::vector<Point> GetNeighbours(const Point& pos) const;
  void ShuffleCheckpoints(const size_t seed = 42);
  void ApplyPermutation(
      const std::vector<size_t>& eject_checkpoints_permutation,
      const std::vector<size_t>& induct_checkpoints_permutation);

private:
  int width;
  int height;
  std::set<Point> obstacles;
  std::vector<Point> eject_checkpoints;
  std::vector<Point> induct_checkpoints;
};
