#pragma once

#include <iostream>
#include <utility>

struct Point {
  Point(const std::pair<int, int>& position);
  Point(const int pos_x, const int pos_y);

  bool operator == (const Point& other) const;
  bool operator < (const Point& other) const;

  int x;
  int y;
};

std::ostream& operator << (std::ostream& ostream, const Point& point);
