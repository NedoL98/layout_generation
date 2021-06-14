#pragma once

#include <iostream>
#include <numeric>
#include <optional>
#include <vector>
#include <utility>

struct Conflict {
  size_t agent_1;
  size_t agent_2;
  size_t ts;
};

struct Point {
  Point() = default;
  Point(const std::pair<int, int>& position);
  Point(const int pos_x, const int pos_y);

  bool operator == (const Point& other) const;
  bool operator < (const Point& other) const;

  int x;
  int y;
};

std::ostream& operator << (std::ostream& ostream, const Point& point);

size_t CalculateCost(const std::vector<std::vector<Point>>& paths);

std::optional<Conflict> FindFirstConflict(
    const std::vector<std::vector<Point>>& paths,
    const std::optional<size_t>& window_size);

struct Assignment {
  size_t start_checkpoint_idx;
  size_t finish_checkpoint_idx;

  Assignment(const size_t start_checkpoint_idx_, const size_t finish_checkpoint_idx_)
    : start_checkpoint_idx(start_checkpoint_idx_)
    , finish_checkpoint_idx(finish_checkpoint_idx_) {}
};

std::ostream& operator << (std::ostream& ostream, const Assignment& assignment);
