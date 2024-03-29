#pragma once

#include <set>
#include <utility>

#include "common.h"
#include "yaml-cpp/yaml.h"

class Graph {
public:
  Graph() = default;
  Graph(const YAML::Node& yaml_graph);
  Graph(const std::string& filename, const double deleted_eject_checkpoints_ratio);

  const std::vector<Point>& GetEjectCheckpoints() const;
  const std::vector<Point>& GetInductCheckpoints() const;
  const std::vector<Point> GetSpareLocations() const;

  std::vector<Point> GetNeighbours(const Point& pos, const bool with_pos = true) const;
  std::optional<Point> GetAnyNearSpareLocation(const Point& pos) const;
  size_t GetTimeToWaitNearCheckpoints() const;
  void ShuffleCheckpoints(const size_t seed = 42);
  void ApplyPermutation(
      const std::vector<size_t>& eject_checkpoints_permutation,
      const std::vector<size_t>& induct_checkpoints_permutation);

  void SetInductCheckpointsAsObstacles(const std::vector<Assignment>& assignments);
  void SetEjectCheckpointsAsObstacles(const std::vector<Assignment>& assignments);
  void KeepOnlySelectedCheckpoints(const std::vector<size_t>& eject_checkpoint_indices);

  bool IsConnected() const;
  bool AllInductCheckpointsAreReachable() const;

private:
  void DFS(const Point& pos, std::set<Point>& used) const;

  int width;
  int height;
  std::set<Point> obstacles;
  std::vector<Point> eject_checkpoints;
  std::vector<Point> induct_checkpoints;
  size_t time_to_wait_near_checkpoints = 1;
};
