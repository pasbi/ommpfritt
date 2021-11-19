#include "commands/modifypointscommand.h"
#include "common.h"
#include "scene/scene.h"
#include "objects/path.h"
#include "objects/pathpoint.h"
#include "objects/segment.h"

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
  std::set<Path*> paths;
  for (auto& [ptr, point] : m_data) {
    const auto geometry = ptr->geometry();
    ptr->set_geometry(point);
    point = geometry;
    paths.insert(ptr->path());
    for (auto* buddy : ptr->joined_points()) {
      paths.insert(buddy->path());
    }
  }
  for (auto* path : paths) {
    path->update();
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
                                             Path& path,
                                             std::deque<OwnedLocatedSegment>&& points_to_add)
  : Command(label)
  , m_path(path)
  , m_points_to_add(std::move(points_to_add))
{
  std::sort(m_points_to_add.rbegin(), m_points_to_add.rend());
  assert(std::is_sorted(m_points_to_add.rbegin(), m_points_to_add.rend()));
}

AbstractPointsCommand::AbstractPointsCommand(const QString& label,
                                             Path& path,
                                             std::deque<SegmentView>&& points_to_remove)
  : Command(label)
  , m_path(path)
  , m_points_to_remove(std::move(points_to_remove))
{
  std::sort(m_points_to_remove.rbegin(), m_points_to_remove.rend());
  assert(std::is_sorted(m_points_to_remove.rbegin(), m_points_to_remove.rend()));
}

void AbstractPointsCommand::add()
{
  assert(m_points_to_remove.empty());
  for (auto& located_segment : m_points_to_add) {
    auto located_segment_view = located_segment.insert_into(m_path);
    m_points_to_remove.push_front(located_segment_view);
  }
  m_points_to_add.clear();
  m_path.update();
  m_path.scene()->update_tool();
}

void AbstractPointsCommand::remove()
{
  assert(m_points_to_add.empty());
  for (const auto& segment_view : m_points_to_remove) {
    m_points_to_add.push_front([this, &view=segment_view]() {
      const auto remove_segment = view.size == view.segment->size();
      if (remove_segment) {
        auto owned_segment = m_path.remove_segment(*view.segment);
        return OwnedLocatedSegment{std::move(owned_segment)};
      } else {
        auto points = view.segment->extract(view.index, view.size);
        return OwnedLocatedSegment{view.segment, view.index, std::move(points)};
      }
    }());
  }
  m_points_to_remove.clear();
  m_path.update();
  m_path.scene()->update_tool();
}

AddPointsCommand::AddPointsCommand(Path& path, std::deque<OwnedLocatedSegment>&& added_points)
    : AbstractPointsCommand(static_label(), path, std::move(added_points))
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

RemovePointsCommand::RemovePointsCommand(Path& path, std::deque<SegmentView>&& removed_points)
    : AbstractPointsCommand(QObject::tr("RemovePointsCommand"), path, std::move(removed_points))
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

AbstractPointsCommand::OwnedLocatedSegment::
OwnedLocatedSegment(Segment* segment, std::size_t index, std::deque<std::unique_ptr<PathPoint> >&& points)
  : m_segment(segment), m_index(index), m_points(std::move(points))
{
  assert(m_segment != nullptr);
  assert(index <= segment->size());
  assert(!m_points.empty());
}

AbstractPointsCommand::OwnedLocatedSegment::OwnedLocatedSegment(std::unique_ptr<Segment> segment)
  : m_owned_segment(std::move(segment)), m_index(0)
{
  assert(m_owned_segment != nullptr);
}

SegmentView AbstractPointsCommand::OwnedLocatedSegment::insert_into(Path& path)
{
  if (m_segment == nullptr) {
    auto& segment = path.add_segment(std::move(m_owned_segment));
    return SegmentView{segment, 0, segment.size()};
  } else {
    const auto n_points = m_points.size();
    m_segment->insert_points(m_index, std::move(m_points));
    return SegmentView{*m_segment, m_index, n_points};
  }
}

bool operator<(const AbstractPointsCommand::OwnedLocatedSegment& a,
               const AbstractPointsCommand::OwnedLocatedSegment& b)
{
  static constexpr auto as_tuple = [](const auto& ola) {
    return std::tuple{ola.m_segment, ola.m_owned_segment.get(), ola.m_index};
  };

  return as_tuple(a) < as_tuple(b);
}

}  // namespace omm
