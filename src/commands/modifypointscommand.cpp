#include "commands/modifypointscommand.h"
#include "common.h"
#include "scene/scene.h"
#include "objects/pathobject.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/pathview.h"

namespace omm
{

ModifyPointsCommand ::ModifyPointsCommand(const std::map<PathPoint*, Point>& points)
    : Command(QObject::tr("ModifyPointsCommand")), m_data(points)
{
  assert(!points.empty());
}

void ModifyPointsCommand::undo()
{
  exchange();
}

void ModifyPointsCommand::redo()
{
  exchange();
}

int ModifyPointsCommand::id() const
{
  return Command::MODIFY_TANGENTS_COMMAND_ID;
}

void ModifyPointsCommand::exchange()
{
  std::set<PathVector*> path_vectors;
  for (auto& [ptr, point] : m_data) {
    const auto geometry = ptr->geometry();
    ptr->set_geometry(point);
    point = geometry;
    path_vectors.insert(ptr->path_vector());
    for (auto* buddy : ptr->joined_points()) {
      path_vectors.insert(buddy->path_vector());
    }
  }
  for (auto* path_vector : path_vectors) {
    path_vector->path_object()->update();
  }
}

bool ModifyPointsCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& mtc = dynamic_cast<const ModifyPointsCommand&>(*command);
  return ::get_keys(m_data) == ::get_keys(mtc.m_data);
}

bool ModifyPointsCommand::is_noop() const
{
  return std::all_of(m_data.begin(), m_data.end(), [](const auto& arg) {
    const auto& [ptr, new_value] = arg;
    return ptr->geometry() == new_value;
  });
}

AbstractPointsCommand::AbstractPointsCommand(const QString& label,
                                             PathObject& path_object,
                                             std::deque<OwnedLocatedPath>&& points_to_add)
  : Command(label)
  , m_path_object(path_object)
  , m_points_to_add(std::move(points_to_add))
{
  std::sort(m_points_to_add.rbegin(), m_points_to_add.rend());
  assert(std::is_sorted(m_points_to_add.rbegin(), m_points_to_add.rend()));
}

AbstractPointsCommand::AbstractPointsCommand(const QString& label,
                                             PathObject& path_object,
                                             std::deque<PathView>&& points_to_remove)
  : Command(label)
  , m_path_object(path_object)
  , m_points_to_remove(std::move(points_to_remove))
{
  std::sort(m_points_to_remove.rbegin(), m_points_to_remove.rend());
  assert(std::is_sorted(m_points_to_remove.rbegin(), m_points_to_remove.rend()));
}

void AbstractPointsCommand::add()
{
  assert(m_points_to_remove.empty());
  for (auto& located_path : m_points_to_add) {
    auto located_path_view = located_path.insert_into(m_path_object.geometry());
    m_points_to_remove.push_front(located_path_view);
  }
  m_points_to_add.clear();
  m_path_object.update();
  m_path_object.scene()->update_tool();
}

void AbstractPointsCommand::remove()
{
  assert(m_points_to_add.empty());
  for (const auto& segment_view : m_points_to_remove) {
    m_points_to_add.push_front([this, &view=segment_view]() {
      const auto remove_segment = view.size == view.path->size();
      if (remove_segment) {
        auto owned_segment = m_path_object.geometry().remove_path(*view.path);
        return OwnedLocatedPath{std::move(owned_segment)};
      } else {
        auto points = view.path->extract(view.index, view.size);
        return OwnedLocatedPath{view.path, view.index, std::move(points)};
      }
    }());
  }
  m_points_to_remove.clear();
  m_path_object.update();
  m_path_object.scene()->update_tool();
}

AbstractPointsCommand::~AbstractPointsCommand() = default;

AddPointsCommand::AddPointsCommand(PathObject& path_object, std::deque<OwnedLocatedPath>&& added_points)
    : AbstractPointsCommand(static_label(), path_object, std::move(added_points))
{
}

void AddPointsCommand::redo()
{
  add();
}

void AddPointsCommand::undo()
{
  remove();
}

QString AddPointsCommand::static_label()
{
  return QObject::tr("AddPointsCommand");
}

RemovePointsCommand::RemovePointsCommand(PathObject& path_object, std::deque<PathView>&& removed_points)
    : AbstractPointsCommand(QObject::tr("RemovePointsCommand"), path_object, std::move(removed_points))
{
}

void RemovePointsCommand::redo()
{
  remove();
}

void RemovePointsCommand::undo()
{
  add();
}

AbstractPointsCommand::OwnedLocatedPath::
OwnedLocatedPath(Path* path, std::size_t index, std::deque<std::unique_ptr<PathPoint> >&& points)
  : m_path(path), m_index(index), m_points(std::move(points))
{
  assert(m_path != nullptr);
  assert(index <= path->size());
  assert(!m_points.empty());
}

AbstractPointsCommand::OwnedLocatedPath::OwnedLocatedPath(std::unique_ptr<Path> path)
  : m_owned_path(std::move(path)), m_index(0)
{
  assert(m_owned_path != nullptr);
}

AbstractPointsCommand::OwnedLocatedPath::~OwnedLocatedPath() = default;

PathView AbstractPointsCommand::OwnedLocatedPath::insert_into(PathVector& path_vector)
{
  if (m_path == nullptr) {
    auto& path = path_vector.add_path(std::move(m_owned_path));
    return PathView{path, 0, path.size()};
  } else {
    const auto n_points = m_points.size();
    m_path->insert_points(m_index, std::move(m_points));
    return PathView{*m_path, m_index, n_points};
  }
}

bool operator<(const AbstractPointsCommand::OwnedLocatedPath& a,
               const AbstractPointsCommand::OwnedLocatedPath& b)
{
  static constexpr auto as_tuple = [](const auto& ola) {
    return std::tuple{ola.m_path, ola.m_owned_path.get(), ola.m_index};
  };

  // NOLINTNEXTLINE(modernize-use-nullptr)
  return as_tuple(a) < as_tuple(b);
}

}  // namespace omm
