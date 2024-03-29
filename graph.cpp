#include "graph.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>

Graph::Graph(const YAML::Node& yaml_graph) {
  width = yaml_graph["dimensions"].as<std::pair<int, int>>().first;
  height = yaml_graph["dimensions"].as<std::pair<int, int>>().second;
  for (const auto& obstacle : yaml_graph["obstacles"]) {
    obstacles.insert(Point{obstacle.as<std::pair<int, int>>()});
  }
}

Graph::Graph(const std::string& filename, const double deleted_eject_checkpoints_ratio) {
  std::ifstream graph_file(filename);
  std::string line;
  std::getline(graph_file, line);
  width = std::stoi(line.substr(0, line.find(',')));
  height = std::stoi(line.substr(line.find(',') + 1, line.size()));
  while (std::getline(graph_file, line)) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = line.find(',');
    while (end != std::string::npos) {
      tokens.push_back(line.substr(start, end - start));
      start = end + 1;
      end = line.find(',', start);
    }
    ASSERT(tokens.size() == 9 && "tokens number is incorrect");
    Point current_point(std::stoi(tokens[3]), std::stoi(tokens[4]));
    if (tokens[1] == "Obstacle") {
      obstacles.insert(current_point);
    } else if (tokens[1] == "Eject") {
      eject_checkpoints.push_back(current_point);
    } else if (tokens[1] == "Induct") {
      obstacles.insert(current_point);
      induct_checkpoints.push_back(current_point);
    } else if (tokens[1] == "Travel") {
      // pass
    } else {
      std::cerr << "Unknown cell type : " << tokens[1] << std::endl;
      exit(0);
    }
  }
  if (deleted_eject_checkpoints_ratio < 1.0) {
    std::vector<size_t> kept_eject_checkpoints_idx(eject_checkpoints.size());
    std::iota(kept_eject_checkpoints_idx.begin(), kept_eject_checkpoints_idx.end(), 0);
    std::random_shuffle(kept_eject_checkpoints_idx.begin(), kept_eject_checkpoints_idx.end());
    kept_eject_checkpoints_idx.resize(
        kept_eject_checkpoints_idx.size() * deleted_eject_checkpoints_ratio);
    std::vector<Point> eject_checkpoints_tmp = std::move(eject_checkpoints);
    eject_checkpoints.clear();
    eject_checkpoints.reserve(kept_eject_checkpoints_idx.size());
    for (size_t i = 0; i < kept_eject_checkpoints_idx.size(); ++i) {
      eject_checkpoints.push_back(eject_checkpoints_tmp[kept_eject_checkpoints_idx[i]]);
    }
  }
}

const std::vector<Point>& Graph::GetEjectCheckpoints() const {
  return eject_checkpoints;
}

const std::vector<Point>& Graph::GetInductCheckpoints() const {
  return induct_checkpoints;
}

const std::vector<Point> Graph::GetSpareLocations() const {
  std::vector<Point> result;
  result.reserve(width * height - obstacles.size());
  for (int i = 0; i < width; ++i) {
    for (int j = 0; j < height; ++j) {
      if (!obstacles.count({i, j})) {
        result.push_back({i, j});
      }
    }
  }
  return result;
}

std::vector<Point> Graph::GetNeighbours(const Point& pos, const bool with_pos) const {
  std::vector<Point> neighbours;
  for (const int dx : {-1, 0, 1}) {
    for (const int dy : {-1, 0, 1}) {
      if (abs(dx) + abs(dy) <= 1
          && pos.x + dx < width && pos.x + dx >= 0
          && pos.y + dy < height && pos.y + dy >= 0
          && !obstacles.count(Point{pos.x + dx, pos.y + dy})
          && (with_pos || abs(dx) + abs(dy) == 1)) {
        neighbours.push_back(Point{pos.x + dx, pos.y + dy});
      }
    }
  }
  return neighbours;
}

std::optional<Point> Graph::GetAnyNearSpareLocation(const Point& pos) const {
  const std::vector<Point> neighbours = GetNeighbours(pos, false);
  if (neighbours.empty()) {
    return std::nullopt;
  } else {
    // This is done for the sake of reproducibility
    return neighbours.front();
  }
}

size_t Graph::GetTimeToWaitNearCheckpoints() const {
  return time_to_wait_near_checkpoints;
}

void Graph::ShuffleCheckpoints(const size_t seed) {
  srand(seed);
  std::random_shuffle(eject_checkpoints.begin(), eject_checkpoints.end());
  std::random_shuffle(induct_checkpoints.begin(), induct_checkpoints.end());
}

void Graph::ApplyPermutation(
    const std::vector<size_t>& eject_checkpoints_permutation,
    const std::vector<size_t>& induct_checkpoints_permutation) {
  const auto sort_and_apply_permutation = [] (
      std::vector<Point>& vec, const std::vector<size_t>& permutation) {
    ASSERT(vec.size() == permutation.size());
    std::sort(vec.begin(), vec.end());
    // todo : make this more efficient
    std::vector<Point> new_vec(vec.size());
    for (size_t i = 0; i < permutation.size(); ++i) {
      new_vec[i] = vec[permutation[i]];
    }
    vec = std::move(new_vec);
  };

  sort_and_apply_permutation(induct_checkpoints, induct_checkpoints_permutation);
  sort_and_apply_permutation(eject_checkpoints, eject_checkpoints_permutation);
}

void Graph::SetInductCheckpointsAsObstacles(const std::vector<Assignment>& assignments) {
  for (const auto& assignment : assignments) {
    obstacles.insert(induct_checkpoints[assignment.finish_checkpoint_idx]);
  }
}

void Graph::SetEjectCheckpointsAsObstacles(const std::vector<Assignment>& assignments) {
  for (const auto& assignment : assignments) {
    obstacles.insert(eject_checkpoints[assignment.finish_checkpoint_idx]);
  }
}

void Graph::KeepOnlySelectedCheckpoints(const std::vector<size_t>& induct_checkpoint_indices) {
  auto induct_checkpoints_tmp = std::move(induct_checkpoints);
  induct_checkpoints.clear();
  induct_checkpoints.reserve(induct_checkpoint_indices.size());
  for (const auto idx : induct_checkpoint_indices) {
    induct_checkpoints.push_back(induct_checkpoints_tmp[idx]);
  }
  obstacles.clear();
  for (const auto& induct_checkpoint : induct_checkpoints) {
    obstacles.insert(induct_checkpoint);
  }
}

bool Graph::IsConnected() const {
  bool dfs_was_run = false;
  std::set<Point> used;
  for (size_t i = 0; i < width; ++i) {
    for (size_t j = 0; j < height; ++j) {
      Point pos(i, j);
      if (!obstacles.count(pos) && !used.count(pos)) {
        if (!dfs_was_run) {
          DFS(pos, used);
          dfs_was_run = true;
        } else {
          return false;
        }
      }
    }
  }
  return true;
}

bool Graph::AllInductCheckpointsAreReachable() const {
  for (const auto& induct_checkpoint : induct_checkpoints) {
    bool checkpoint_is_reachable = false;
    for (const auto& neighbour : GetNeighbours(induct_checkpoint, false)) {
      if (!obstacles.count(neighbour)) {
        checkpoint_is_reachable = true;
        break;
      }
    }
    if (!checkpoint_is_reachable) {
      return false;
    }
  }
  return true;
}

void Graph::DFS(const Point& pos, std::set<Point>& used) const {
  used.insert(pos);
  for (const auto& neighbour : GetNeighbours(pos, false)) {
    if (!used.count(neighbour)) {
      DFS(neighbour, used);
    }
  }
}