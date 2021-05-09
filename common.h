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
