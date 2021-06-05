#include "common.h"

#include <algorithm>
#include <map>

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

size_t CalculateCost(const std::vector<std::vector<Point>>& paths) {
  return std::accumulate(paths.begin(), paths.end(), 0,
      [](size_t cost, const std::vector<Point>& path) {
          return cost + path.size();
  });
}

std::optional<Conflict> FindFirstConflict(
    const std::vector<std::vector<Point>>& paths,
    const std::optional<size_t>& window_size) {
  size_t max_timestamp = std::max_element(paths.begin(), paths.end(), []
      (const std::vector<Point>& v1, const std::vector<Point>& v2) {
          return v1.size() < v2.size();
  })->size();
  if (window_size) {
    max_timestamp = std::min(max_timestamp, window_size.value());
  }
  std::map<Point, size_t> position_to_agent;
  for (size_t ts = 0; ts < max_timestamp; ++ts) {
    position_to_agent.clear();
    for (size_t agent_id = 0; agent_id < paths.size(); ++agent_id) {
      if (paths[agent_id].size() <= ts) {
        continue;
      }
      const auto agent_pos = paths[agent_id][ts];
      if (position_to_agent.count(agent_pos)) {
        // Conflict found
        std::cerr << "has conflict for : " << agent_id << " and " << position_to_agent.at(agent_pos) << " at " << ts << std::endl;
        return Conflict{position_to_agent.at(agent_pos), agent_id, ts};
      }
      position_to_agent[agent_pos] = agent_id;
    }
  }
  return std::nullopt;
}

std::ostream& operator << (std::ostream& ostream, const Assignment& assignment) {
  ostream << "{" << assignment.start << ", " << assignment.finish << "}";
  return ostream;
}
