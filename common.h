#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <vector>
#include <utility>

#define ASSERT(condition) \
  if (!(condition)) { \
    std::cout << "assertion failed: " \
        << #condition << " @ " \
        << __FILE__ << " (" \
        << __LINE__ << ")" << std::endl; \
    assert(condition); \
  }

struct Point {
  Point() = default;
  Point(const std::pair<int, int>& position);
  Point(const int pos_x, const int pos_y);

  bool operator == (const Point& other) const;
  bool operator < (const Point& other) const;
  bool operator > (const Point& other) const;
  bool operator != (const Point& other) const;

  int x;
  int y;
};

std::ostream& operator << (std::ostream& ostream, const Point& point);

using Edge = std::pair<Point, Point>;

size_t CalculateCost(const std::vector<std::vector<Point>>& paths);
double CalculateThroughput(const std::vector<std::vector<Point>>& paths, const size_t assignments);

struct ConflictBase;

std::shared_ptr<ConflictBase> FindFirstConflict(
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

enum class ConflictType {
  Undefined,
  VertexConflict,
  EdgeConflict
};

struct ConflictBase {
  virtual ~ConflictBase() = default;

  ConflictBase(size_t agent_1_, size_t agent_2_, size_t ts_, ConflictType conflict_type_)
    : agent_1(agent_1_)
    , agent_2(agent_2_)
    , ts(ts_)
    , conflict_type(conflict_type_) {}

  const size_t agent_1;
  const size_t agent_2;
  const size_t ts;
  const ConflictType conflict_type = ConflictType::Undefined;
};

struct VertexConflict : ConflictBase {
  VertexConflict(size_t agent_1_, size_t agent_2_, size_t ts_, const Point& conflicting_vertex_)
    : ConflictBase(agent_1_, agent_2_, ts_, ConflictType::VertexConflict)
    , conflicting_vertex(conflicting_vertex_) {}

  const Point conflicting_vertex;
};

struct EdgeConflict : ConflictBase {
  EdgeConflict(
    size_t agent_1_, size_t agent_2_, size_t ts_, const std::pair<Point, Point>& conflicting_edge_)
    : ConflictBase(agent_1_, agent_2_, ts_, ConflictType::EdgeConflict)
    , conflicting_edge(conflicting_edge_) {}

  const Edge conflicting_edge;
};
