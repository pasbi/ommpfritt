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
    const auto n = located_segment.points.size();
    if (located_segment.segment == nullptr) {
      auto& segment = m_path.add_segment(std::make_unique<Segment>(std::move(located_segment.points)));
      m_points_to_remove.emplace_back(segment, 0, n);
    } else {
      located_segment.segment->insert_points(located_segment.index, std::move(located_segment.points));
      m_points_to_remove.emplace_back(*located_segment.segment, located_segment.index, n);
    }
  }
  m_points_to_add.clear();
  m_path.update();
  m_path.scene()->update_tool();
}

void AbstractPointsCommand::remove()
{
//  std::list<LocatedSegment> points_to_add;
//  for (auto&& range : m_points_to_remove) {
//    assert(range.length > 0);
//    auto& segment = m_path.segments[range.begin.segment];
//    if (range.length == segment.size()) {
//      assert(range.begin.point == 0);
//      points_to_add.push_front(LocatedSegment{range.begin, m_path.segments[range.begin.segment]});
//      m_path.segments.erase(m_path.segments.begin() + range.begin.segment);
//    } else {
//      auto begin = segment.begin() + range.begin.point;
//      auto end = begin + range.length;
//      Segment extracted_segment;
//      extracted_segment.reserve(range.length);
//      std::copy(begin, end, std::back_inserter(extracted_segment));
//      segment.erase(begin, end);
//      points_to_add.push_front(LocatedSegment{range.begin, extracted_segment});
//    }
//  }
//  m_points_to_add = std::vector(points_to_add.begin(), points_to_add.end());
//  m_path.update();
//  m_path.scene()->update_tool();
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

}  // namespace omm
