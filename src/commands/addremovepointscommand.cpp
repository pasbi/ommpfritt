#include "commands/addremovepointscommand.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathview.h"
#include "transform.h"
#include <memory>

namespace omm
{

class AddRemovePointsCommand::ChangeSet
{
public:
  explicit ChangeSet(const PathView& view, std::deque<std::unique_ptr<Edge>> edges)
    : m_view(view)
    , m_other(std::move(edges))
  {
  }

  void swap()
  {
    PathView view2{m_view.path(), m_view.begin(), m_other.size()};
    m_other = m_view.path().replace(m_view, std::move(m_other));
    m_view = view2;
  }

private:
  PathView m_view;
  std::deque<std::unique_ptr<Edge>> m_other;
};

OwnedLocatedPath::~OwnedLocatedPath() = default;

OwnedLocatedPath::OwnedLocatedPath(Path* path, std::size_t index, std::deque<std::unique_ptr<PathPoint>> points)
    : m_path(path)
    , m_index(index)
    , m_points(std::move(points))
{
}

std::deque<std::unique_ptr<Edge> > OwnedLocatedPath::create_edges()
{
  std::deque<std::unique_ptr<Edge>> edges;
  for (std::size_t i = 1; i < m_points.size(); ++i) {
    edges.emplace_back(std::make_unique<Edge>(std::move(m_points[i - 1]), std::move(m_points[i]), m_path));
  }

  const auto points = m_path->points();

  if (m_index > 0) {
    // if there is something left of this, add the linking edge
    auto right_fringe = m_path->edges()[m_index]->b();
    edges.emplace_front(std::make_unique<Edge>(right_fringe, edges.front()->a(), m_path));
  }

  if (const auto index = m_index + m_points.size(); index + 1 < points.size()) {
    // if there is something right of this, add the linking edge
    auto left_fringe = m_path->edges()[index]->a();
    edges.emplace_back(std::make_unique<Edge>(edges.back()->b(), left_fringe, m_path));
  }

  return edges;
}

PathView OwnedLocatedPath::path_view() const
{
  return PathView{*m_path, m_index, m_points.size()};
}

}  // namespace omm

namespace
{

auto make_change_set_for_add(omm::OwnedLocatedPath points_to_add)
{
  return omm::AddRemovePointsCommand::ChangeSet{points_to_add.path_view(), points_to_add.create_edges()};
}

auto make_change_set_for_remove(const omm::PathView& path_view)
{
  std::deque<std::unique_ptr<omm::Edge>> edges;
  auto& path = path_view.path();
  auto& left = *path.edges().at(path_view.begin());
  auto& right = *path.edges().at(path_view.end());
  edges.emplace_back(std::make_unique<omm::Edge>(left.b(), right.a(), &path));
  return omm::AddRemovePointsCommand::ChangeSet{path_view, std::move(edges)};
}

}  // namespace

namespace omm
{

AddRemovePointsCommand::AddRemovePointsCommand(const QString& label, std::deque<ChangeSet> changes)
    : Command(label)
    , m_change_sets(std::move(changes))
{
}

AddRemovePointsCommand::~AddRemovePointsCommand() = default;

void AddRemovePointsCommand::restore_bridges()
{
  std::for_each(m_change_sets.begin(), m_change_sets.end(), [](auto& cs) { cs.swap(); });
}

void AddRemovePointsCommand::restore_edges()
{
  std::for_each(m_change_sets.rbegin(), m_change_sets.rend(), [](auto& cs) { cs.swap(); });
}

AddPointsCommand::AddPointsCommand(std::deque<OwnedLocatedPath> points_to_add)
    : AddRemovePointsCommand(static_label(), util::transform(std::move(points_to_add), make_change_set_for_add))
{
}

void AddPointsCommand::undo()
{
  restore_bridges();
}

void AddPointsCommand::redo()
{
  restore_edges();
}

QString AddPointsCommand::static_label()
{
  return QObject::tr("AddPointsCommand");
}

RemovePointsCommand::RemovePointsCommand(const std::deque<PathView>& points_to_remove)
    : AddRemovePointsCommand(static_label(), util::transform(points_to_remove, make_change_set_for_remove))
{
}

void RemovePointsCommand::undo()
{
  restore_edges();
}

void RemovePointsCommand::redo()
{
  restore_bridges();
}

QString RemovePointsCommand::static_label()
{
  return QObject::tr("RemovePointsCommand");
}

}  // namespace omm
