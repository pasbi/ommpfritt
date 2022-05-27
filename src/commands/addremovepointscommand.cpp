#include "commands/addremovepointscommand.h"
#include "logging.h"
#include "objects/pathobject.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathview.h"
#include "scene/scene.h"
#include "transform.h"
#include <memory>

namespace omm
{

class AddRemovePointsCommand::ChangeSet
{
public:
  explicit ChangeSet(const PathView& view, std::deque<std::unique_ptr<Edge>> edges)
    : m_view(view)
    , m_owned_edges(std::move(edges))
  {
  }

  void swap()
  {
    PathView view2{m_view.path(), m_view.begin(), m_owned_edges.size()};
    LINFO << "RUHURHG" << (void*) &m_view.path() << "\n\n";
    std::cout << std::endl;
    m_owned_edges = m_view.path().replace(m_view, std::move(m_owned_edges));
    m_view = view2;
  }

  std::vector<Edge*> owned_edges() const
  {
    return util::transform<std::vector>(m_owned_edges, &std::unique_ptr<Edge>::get);
  }

private:
  PathView m_view;
  std::deque<std::unique_ptr<Edge>> m_owned_edges;
};

OwnedLocatedPath::~OwnedLocatedPath() = default;

OwnedLocatedPath::OwnedLocatedPath(Path* const path, const std::size_t point_offset, std::deque<std::shared_ptr<PathPoint>> points)
    : m_path(path)
    , m_point_offset(point_offset)
    , m_points(std::move(points))
{
}

std::deque<std::unique_ptr<Edge> > OwnedLocatedPath::create_edges()
{
  std::deque<std::unique_ptr<Edge>> edges;
  for (std::size_t i = 1; i < m_points.size(); ++i) {
    edges.emplace_back(std::make_unique<Edge>(std::move(m_points[i - 1]), std::move(m_points[i]), m_path));
  }

  std::shared_ptr<PathPoint> front = edges.empty() ? std::move(m_points.front()) : edges.front()->a();
  std::shared_ptr<PathPoint> back = edges.empty() ? std::move(m_points.back()) : edges.back()->b();

  const auto points = m_path->points();

  if (m_point_offset > 0) {
    // if there is something left of this, add the linking edge
    auto right_fringe = m_path->edges()[m_point_offset - 1]->b();
    edges.emplace_front(std::make_unique<Edge>(right_fringe, front, m_path));
  }

  if (const auto index = m_point_offset + m_points.size(); index + 1 < points.size()) {
    // if there is something right of this, add the linking edge
    auto left_fringe = m_path->edges()[index]->a();
    edges.emplace_back(std::make_unique<Edge>(back, left_fringe, m_path));
  }

  return edges;
}

std::size_t OwnedLocatedPath::point_offset() const
{
  return m_point_offset;
}

Path* OwnedLocatedPath::path() const
{
  return m_path;
}

}  // namespace omm

namespace
{

auto make_change_set_for_add(omm::OwnedLocatedPath points_to_add)
{
  omm::PathView path_view_to_remove{*points_to_add.path(), points_to_add.point_offset(), 0};
  return omm::AddRemovePointsCommand::ChangeSet{path_view_to_remove, points_to_add.create_edges()};
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

AddRemovePointsCommand::AddRemovePointsCommand(const QString& label,
                                               PathObject& path_object,
                                               std::deque<ChangeSet> changes)
    : Command(label)
    , m_change_sets(std::move(changes))
    , m_path_object(path_object)
{
}

AddRemovePointsCommand::~AddRemovePointsCommand() = default;

void AddRemovePointsCommand::restore_bridges()
{
  std::for_each(m_change_sets.begin(), m_change_sets.end(), [](auto& cs) { cs.swap(); });
  update();
}

void AddRemovePointsCommand::restore_edges()
{
  std::for_each(m_change_sets.rbegin(), m_change_sets.rend(), [](auto& cs) { cs.swap(); });
  update();
}

std::deque<Edge*> AddRemovePointsCommand::owned_edges() const
{
  std::deque<Edge*> new_edges;
  for (const auto& cs : m_change_sets) {
    const auto& oe = cs.owned_edges();
    new_edges.insert(new_edges.end(), oe.begin(), oe.end());
  }
  return new_edges;
}

void AddRemovePointsCommand::update()
{
  m_path_object.update();
  m_path_object.scene()->update_tool();
}

AddPointsCommand::AddPointsCommand(PathObject& path_object, std::deque<OwnedLocatedPath> points_to_add)
    : AddRemovePointsCommand(static_label(),
                             path_object,
                             util::transform(std::move(points_to_add), make_change_set_for_add))
    , m_new_edges(owned_edges())  // owned_edges are new edges before calling redo.
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

std::deque<Edge*> AddPointsCommand::new_edges() const
{
  return m_new_edges;
}

RemovePointsCommand::RemovePointsCommand(PathObject& path_object, const std::deque<PathView>& points_to_remove)
    : AddRemovePointsCommand(static_label(),
                             path_object,
                             util::transform(points_to_remove, make_change_set_for_remove))
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
