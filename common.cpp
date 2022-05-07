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

bool Point::operator > (const Point& other) const {
  return !(*this < other) && !(*this == other);
}

bool Point::operator != (const Point& other) const {
  return !(*this == other);
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


size_t CalculateMaxLength(const std::vector<std::vector<Point>>& paths) {
  return std::max_element(paths.begin(), paths.end(),
      [](const std::vector<Point>& lhs, const std::vector<Point>& rhs) {
    return lhs.size() < rhs.size();
  })->size();
}

double CalculateThroughput(const std::vector<std::vector<Point>>& paths, const size_t assignments) {
  return static_cast<double>(assignments) / CalculateMaxLength(paths);
}

std::shared_ptr<ConflictBase> FindFirstConflict(
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
  std::map<Edge, size_t> edge_to_agent;
  // todo : ts == 0 breaks the case when one agent is done
  // and another one is trying to go through it. Fix this
  for (size_t ts = 1; ts < max_timestamp; ++ts) {
    position_to_agent.clear();
    edge_to_agent.clear();
    for (size_t agent_id = 0; agent_id < paths.size(); ++agent_id) {
      if (paths[agent_id].size() <= ts) {
        continue;
      }
      const auto agent_pos = paths[agent_id][ts];
      if (position_to_agent.count(agent_pos)) {
        // Vertex conflict found
        std::cerr << "has vertex conflict for : " << agent_id << " and " << position_to_agent.at(agent_pos) << std::endl;
        std::cerr << "ts: " << ts << std::endl;
        std::cerr << "vertex : " << agent_pos << std::endl;
        return std::make_shared<VertexConflict>(
            VertexConflict(position_to_agent.at(agent_pos), agent_id, ts, agent_pos));
      }
      position_to_agent[agent_pos] = agent_id;

      if (ts > 0) {
        const Edge edge = {paths[agent_id][ts - 1], agent_pos};
        const Edge rev_edge = {agent_pos, paths[agent_id][ts - 1]};
        if (edge_to_agent.count(rev_edge)) {
          // Edge conflict found
          std::cerr << "has edge conflict for : " << agent_id << " and " << edge_to_agent.at(rev_edge) << std::endl;
          std::cerr << "ts: " << ts << std::endl;
          std::cerr << "edge : {" << edge.first << ", " << edge.second << "}" << std::endl;
          return std::make_shared<EdgeConflict>(
              EdgeConflict{edge_to_agent.at(rev_edge), agent_id, ts, rev_edge});
        }
        edge_to_agent[edge] = agent_id;
      }
    }
  }
  return nullptr;
}

std::ostream& operator << (std::ostream& ostream, const Assignment& assignment) {
  ostream << "{" << assignment.start_checkpoint_idx
          << ", " << assignment.finish_checkpoint_idx << "}";
  return ostream;
}
