#include "commands/modifypointscommand.h"
#include "common.h"
#include "scene/scene.h"
#include "objects/path.h"
#include "objects/segment.h"

namespace omm
{

//ModifyPointsCommand ::ModifyPointsCommand(const std::map<PathIterator, Point>& points)
//    : Command(QObject::tr("ModifyPointsCommand")), m_data(points)
//{
//  assert(!points.empty());
//}

//void ModifyPointsCommand::undo()
//{
//  swap();
//}
//void ModifyPointsCommand::redo()
//{
//  swap();
//}
//int ModifyPointsCommand::id() const
//{
//  return Command::MODIFY_TANGENTS_COMMAND_ID;
//}

//void ModifyPointsCommand::swap()
//{
//  std::set<Path*> paths;
//  for (auto& [it, point] : m_data) {
//    it->swap(point);
//    paths.insert(it.path);
//  }
//  for (Path* path : paths) {
//    path->update();
//  }
//}

//bool ModifyPointsCommand::mergeWith(const QUndoCommand* command)
//{
//  // merging happens automatically!
//  const auto& mtc = dynamic_cast<const ModifyPointsCommand&>(*command);
//  return ::get_keys(m_data) == ::get_keys(mtc.m_data);
//}

AbstractPointsCommand::AbstractPointsCommand(const QString& label, Path& path, std::deque<OwnedLocatedSegment>&& points_to_add)
  : Command(label)
  , m_path(path)
  , m_points_to_add(std::move(points_to_add))
{
}

AbstractPointsCommand::AbstractPointsCommand(const QString& label, Path& path, const std::deque<LocatedSegmentView>& points_to_remove)
  : Command(label)
  , m_path(path)
  , m_points_to_remove(points_to_remove)
{
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
    auto located_segment = segment_view.extract_from(m_path);
    m_points_to_add.push_back(std::move(located_segment));
  }
  m_points_to_remove.clear();
  m_path.update();
  m_path.scene()->update_tool();
}

AddPointsCommand::AddPointsCommand(Path& path, std::deque<OwnedLocatedSegment>&& added_points)
    : AbstractPointsCommand(QObject::tr("AddPointsCommand"), path, std::move(added_points))
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

RemovePointsCommand::RemovePointsCommand(Path& path, const std::deque<LocatedSegmentView>& removed_points)
    : AbstractPointsCommand(QObject::tr("RemovePointsCommand"), path, removed_points)
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

AbstractPointsCommand::LocatedSegmentView::LocatedSegmentView(Segment& segment, std::size_t index, std::size_t size)
  : m_segment(segment), m_index(index), m_size(size)
{
}

AbstractPointsCommand::OwnedLocatedSegment AbstractPointsCommand::LocatedSegmentView::extract_from(Path& path) const
{
  const auto remove_segment = m_size == m_segment.size();
  if (remove_segment) {
    auto owned_segment = path.remove_segment(m_segment);
    return OwnedLocatedSegment{std::move(owned_segment)};
  } else {
    auto points = m_segment.extract(m_index, m_size);
    return OwnedLocatedSegment{&m_segment, m_index, std::move(points)};
  }
}

AbstractPointsCommand::OwnedLocatedSegment::OwnedLocatedSegment(Segment* segment, std::size_t index, std::deque<std::unique_ptr<Point> >&& points)
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

AbstractPointsCommand::LocatedSegmentView AbstractPointsCommand::OwnedLocatedSegment::insert_into(Path& path)
{
  if (m_segment == nullptr) {
    auto& segment = path.add_segment(std::move(m_owned_segment));
    return LocatedSegmentView{segment, 0, segment.size()};
  } else {
    const auto n_points = m_points.size();
    m_segment->insert_points(m_index, std::move(m_points));
    return LocatedSegmentView{*m_segment, m_index, n_points};
  }
}

}  // namespace omm
