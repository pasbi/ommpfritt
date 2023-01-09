#pragma once

#include "path/pathview.h"
#include <memory>
#include <vector>

namespace omm
{

class PathPoint;
class Edge;

class AddRemovePointsCommandChangeSet
{
public:
  explicit AddRemovePointsCommandChangeSet(const PathView& view, std::deque<std::unique_ptr<Edge>> edges,
                                           std::shared_ptr<PathPoint> single_point);

  void swap();
  std::vector<Edge*> owned_edges() const;

private:
  PathView m_view;
  std::deque<std::unique_ptr<Edge>> m_owned_edges;
  std::shared_ptr<PathPoint> m_owned_point;
};

}  // namespace omm
