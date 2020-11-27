#include "commands/modifypointscommand.h"
#include "common.h"
#include "scene/scene.h"

namespace omm
{
ModifyPointsCommand ::ModifyPointsCommand(const std::map<Path::iterator, Point>& points)
    : Command(QObject::tr("ModifyPointsCommand")), m_data(points)
{
  assert(!points.empty());
}

void ModifyPointsCommand::undo()
{
  swap();
}
void ModifyPointsCommand::redo()
{
  swap();
}
int ModifyPointsCommand::id() const
{
  return Command::MODIFY_TANGENTS_COMMAND_ID;
}

void ModifyPointsCommand::swap()
{
  std::set<Path*> paths;
  for (auto& [it, point] : m_data) {
    it->swap(point);
    paths.insert(it.path);
  }
  for (Path* path : paths) {
    path->update();
  }
}

bool ModifyPointsCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& mtc = dynamic_cast<const ModifyPointsCommand&>(*command);
  return ::get_keys(m_data) == ::get_keys(mtc.m_data);
}

AbstractPointsCommand::AbstractPointsCommand(
    const QString& label,
    Path& path,
    const std::vector<AbstractPointsCommand::LocatedSegment>& added_points)
    : Command(label), m_path(path), m_points_to_add(added_points)
{
  std::sort(m_points_to_add.begin(), m_points_to_add.end(), std::greater<LocatedSegment>());
}

AbstractPointsCommand::AbstractPointsCommand(const QString& label,
                                             Path& path,
                                             const std::vector<Range>& removed_points)
    : Command(label), m_path(path), m_points_to_remove(removed_points)
{
  std::sort(m_points_to_remove.begin(), m_points_to_remove.end(), std::greater<Range>());
}

void AbstractPointsCommand::add()
{
  std::list<Range> points_to_remove;
  for (auto&& [location, points_to_add] : m_points_to_add) {
    if (location.is_end()) {
      m_path.segments.push_back(points_to_add);
    } else {
      auto& segment = m_path.segments[location.segment];
      const auto it = segment.begin() + location.point;
      segment.insert(it, points_to_add.begin(), points_to_add.end());
    }
    points_to_remove.push_front(Range{location, points_to_add.size()});
  }
  m_points_to_remove = std::vector(points_to_remove.begin(), points_to_remove.end());
  m_path.update();
  m_path.scene()->update_tool();
}

void AbstractPointsCommand::remove()
{
  std::list<LocatedSegment> points_to_add;
  for (auto&& range : m_points_to_remove) {
    assert(range.length > 0);
    auto& segment = m_path.segments[range.begin.segment];
    if (range.length == segment.size()) {
      assert(range.begin.point == 0);
      points_to_add.push_front(LocatedSegment{range.begin, m_path.segments[range.begin.segment]});
      m_path.segments.erase(m_path.segments.begin() + range.begin.segment);
    } else {
      auto begin = segment.begin() + range.begin.point;
      auto end = begin + range.length;
      Path::Segment extracted_segment;
      extracted_segment.reserve(range.length);
      std::copy(begin, end, std::back_inserter(extracted_segment));
      segment.erase(begin, end);
      points_to_add.push_front(LocatedSegment{range.begin, extracted_segment});
    }
  }
  m_points_to_add = std::vector(points_to_add.begin(), points_to_add.end());
  m_path.update();
  m_path.scene()->update_tool();
}

AddPointsCommand::AddPointsCommand(
    Path& path,
    const std::vector<AbstractPointsCommand::LocatedSegment>& added_points)
    : AbstractPointsCommand(QObject::tr("AddPointsCommand"), path, added_points)
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

RemovePointsCommand::RemovePointsCommand(Path& path, const std::vector<Range>& removed_points)
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

bool AbstractPointsCommand::LocatedSegment::operator<(const LocatedSegment& other) const
{
  return index < other.index;
}

bool AbstractPointsCommand::LocatedSegment::operator>(const LocatedSegment& other) const
{
  return index > other.index;
}

bool AbstractPointsCommand::Range::intersects(const AbstractPointsCommand::Range& other) const
{
  if (this->begin.segment != other.begin.segment) {
    return false;
  } else {
    return this->begin.point + this->length > other.begin.point;
  }
}

bool AbstractPointsCommand::Range::operator<(const Range& other) const
{
  return begin < other.begin;
}

bool AbstractPointsCommand::Range::operator>(const Range& other) const
{
  return begin > other.begin;
}

}  // namespace omm
