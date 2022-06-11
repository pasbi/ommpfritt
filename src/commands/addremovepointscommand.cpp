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
  explicit ChangeSet(const PathView& view,
                     std::deque<std::unique_ptr<Edge>> edges,
                     std::shared_ptr<PathPoint> single_point)
    : m_view(view)
    , m_owned_edges(std::move(edges))
    , m_owned_point(std::move(single_point))
  {
    assert((m_owned_point == nullptr) || m_owned_edges.empty());
  }

  void swap()
  {
    std::size_t added_point_count = 0;

    if (m_view.path().points().empty() && m_owned_point) {
      // path empty, add single point
      assert(m_owned_edges.empty());
      m_view.path().set_single_point(std::move(m_owned_point));
      added_point_count = 1;
    } else if (m_view.path().points().size() == 1 && m_view.point_count() == 1) {
      // path contains only a single point which is going to be removed
      m_owned_point = m_view.path().extract_single_point();
      added_point_count = 0;
    } else {
      // all other cases are handled by Path::replace
      auto& path = m_view.path();
      if (m_owned_edges.empty()) {
        added_point_count = 0;
      } else if (path.points().empty()) {
        added_point_count = m_owned_edges.size() + 1;
      } else if (m_view.begin() == 0 || m_view.end() == path.points().size()) {
        added_point_count = m_owned_edges.size();
      } else {
        added_point_count = m_owned_edges.size() - 1;
      }
      m_owned_edges = path.replace(m_view, std::move(m_owned_edges));
    }

    m_view = PathView{m_view.path(), m_view.begin(), added_point_count};
  }

  std::vector<Edge*> owned_edges() const
  {
    return util::transform<std::vector>(m_owned_edges, &std::unique_ptr<Edge>::get);
  }

private:
  PathView m_view;
  std::deque<std::unique_ptr<Edge>> m_owned_edges;
  std::shared_ptr<PathPoint> m_owned_point;
};

OwnedLocatedPath::~OwnedLocatedPath() = default;

OwnedLocatedPath::OwnedLocatedPath(Path* const path, const std::size_t point_offset, std::deque<std::shared_ptr<PathPoint>> points)
    : m_path(path)
    , m_point_offset(point_offset)
    , m_points(std::move(points))
{
}

std::deque<std::unique_ptr<Edge>> OwnedLocatedPath::create_edges() const
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
    std::shared_ptr<PathPoint> right_fringe;
    if (m_path->edges().empty()) {
      right_fringe = m_path->last_point();
    } else {
      right_fringe = m_path->edges()[m_point_offset - 1]->a();
    }
    edges.emplace_front(std::make_unique<Edge>(right_fringe, front, m_path));
  }

  if (m_point_offset < m_path->points().size()) {
    // if there is something right of this, add the linking edge
    std::shared_ptr<PathPoint> left_fringe;
    if (m_path->edges().empty()) {
      left_fringe = m_path->first_point();
    } else {
      left_fringe = m_path->edges()[m_point_offset]->a();
    }
    edges.emplace_back(std::make_unique<Edge>(back, left_fringe, m_path));
  }

  return edges;
}

std::shared_ptr<PathPoint> OwnedLocatedPath::single_point() const
{
  if (m_points.size() == 1 && m_path->points().size() == 0) {
    return m_points.front();
  } else {
    return {};
  }
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
  return omm::AddRemovePointsCommand::ChangeSet{path_view_to_remove,
                                                points_to_add.create_edges(),
                                                points_to_add.single_point()};
}

auto make_change_set_for_remove(const omm::PathView& path_view)
{
  std::deque<std::unique_ptr<omm::Edge>> edges;
  auto& path = path_view.path();

  if (path_view.begin() > 0 && path_view.end() < path.edges().size() + 1) {
    auto& left = *path.edges().at(path_view.begin() - 1);
    auto& right = *path.edges().at(path_view.end() - 1);
    edges.emplace_back(std::make_unique<omm::Edge>(left.a(), right.b(), &path));
  }
  return omm::AddRemovePointsCommand::ChangeSet{path_view, std::move(edges), {}};
}

}  // namespace

namespace omm
{

AddRemovePointsCommand::AddRemovePointsCommand(const QString& label,
                                               std::deque<ChangeSet> changes,
                                               PathObject* const path_object)
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
  if (m_path_object != nullptr) {
    m_path_object->update();
    m_path_object->scene()->update_tool();
  }
}

AddPointsCommand::AddPointsCommand(std::deque<OwnedLocatedPath> points_to_add, PathObject* const path_object)
    : AddRemovePointsCommand(static_label(),
                             util::transform(std::move(points_to_add), make_change_set_for_add),
                             path_object)
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

RemovePointsCommand::RemovePointsCommand(const std::deque<PathView>& points_to_remove, PathObject* const path_object)
    : AddRemovePointsCommand(static_label(),
                             util::transform(points_to_remove, make_change_set_for_remove),
                             path_object)
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
