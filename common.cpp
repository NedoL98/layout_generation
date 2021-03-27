#include "common.h"

Point::Point(const std::pair<int, int>& position)
  : x(position.first)
  , y(position.second) {}

Point::Point(const int pos_x, const int pos_y)
  : x(pos_x)
  , y(pos_y) {}

bool Point::operator == (const Point& other) const{
  return x == other.x && y == other.y;
}

bool Point::operator < (const Point& other) const {
  return x < other.x || (x == other.x && y < other.y);
}

std::ostream& operator << (std::ostream& ostream, const Point& point) {
  ostream << "{" << point.x << ", " << point.y << "}";
  return ostream;
}
