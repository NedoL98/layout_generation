#pragma once

#include <utility>
#include <deque>

#include "common.h"
#include "graph.h"
#include "task_assigner.h"
#include "yaml-cpp/yaml.h"

struct Agent {
  Point start;
  std::deque<Point> locations_to_visit;
  std::vector<std::pair<Point, Point>> all_checkpoints;
  size_t id;

  Agent(const Point& start_, const size_t id_)
    : start(start_)
    , id(id_) {}

  void PrintDebugInfo(std::ostream& ostream) const;
  size_t CalculateLowerBound() const;
};

class Agents {
public:
  Agents(const YAML::Node& yaml_agents);
  Agents(const Graph& graph, const size_t agents_num, const size_t seed = 42);

  const std::vector<Agent>& GetAgents() const;
  const Agent& At(const size_t index) const;
  const size_t GetSize() const;

  void UpdateTasksLists(TaskAssigner& task_assigner, const size_t window_size, const Graph& graph);
  bool DeleteCompletedTasks(
      const std::vector<std::vector<Point>>& path_prefixes, const size_t window_size);

private:
  std::vector<Agent> agents;
};
