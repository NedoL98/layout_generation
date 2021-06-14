#include "graph.h"

#include <cassert>
#include <fstream>
#include <iostream>

Graph::Graph(const YAML::Node& yaml_graph) {
  width = yaml_graph["dimensions"].as<std::pair<int, int>>().first;
  height = yaml_graph["dimensions"].as<std::pair<int, int>>().second;
  for (const auto& obstacle : yaml_graph["obstacles"]) {
    obstacles.insert(Point{obstacle.as<std::pair<int, int>>()});
  }
}

Graph::Graph(const char* filename) {
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
    assert(tokens.size() == 9 && "tokens number is incorrect");
    Point current_point(std::stoi(tokens[3]), std::stoi(tokens[4]));
    if (tokens[1] == "Obstacle") {
      obstacles.insert(current_point);
    } else if (tokens[1] == "Eject") {
      eject_checkpoints.push_back(current_point);
    } else if (tokens[1] == "Induct") {
      induct_checkpoints.push_back(current_point);
    } else if (tokens[1] == "Travel") {
      // pass
    } else {
      std::cerr << "Unknown cell type : " << tokens[1] << std::endl;
      exit(0);
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
  for (size_t i = 0; i < width; ++i) {
    for (size_t j = 0; j < height; ++j) {
      if (!obstacles.count({i, j})) {
        result.push_back({i, j});
      }
    }
  }
  return result;
}

std::vector<Point> Graph::GetNeighbours(const Point& pos) const {
  std::vector<Point> neighbours;
  for (const int dx : {-1, 0, 1}) {
    for (const int dy : {-1, 0, 1}) {
      if (abs(dx) + abs(dy) <= 1
          && pos.x + dx < width && pos.x + dx >= 0
          && pos.y + dy < height && pos.y + dy >= 0
          && !obstacles.count(Point{pos.x + dx, pos.y + dy})) {
        neighbours.push_back(Point{pos.x + dx, pos.y + dy});
      }
    }
  }
  return neighbours;
}

void Graph::ShuffleCheckpoints(const size_t seed) {
  srand(seed);
  std::random_shuffle(eject_checkpoints.begin(), eject_checkpoints.end());
  std::random_shuffle(induct_checkpoints.begin(), induct_checkpoints.end());
}
