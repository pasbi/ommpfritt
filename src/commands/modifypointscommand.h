#pragma once

#include "commands/command.h"
#include <deque>
#include <memory>

namespace omm
{

class Path;
class Point;
class Segment;

//class ModifyPointsCommand : public Command
//{
//public:
//  using map_type = std::map<PathIterator, Point>;
//  ModifyPointsCommand(const map_type& points);
//  void redo() override;
//  void undo() override;
//  [[nodiscard]] int id() const override;
//  bool mergeWith(const QUndoCommand* command) override;

//private:
//  std::map<PathIterator, Point> m_data;
//  void swap();
//};

class AbstractPointsCommand : public Command
{
public:
  struct OwnedLocatedSegment
  {
    Segment* segment;
    std::size_t index;
    std::deque<std::unique_ptr<Point>> points;
  };


  struct LocatedSegmentView
  {
    Segment& segment;
    std::size_t index;
    std::size_t size;
  };

protected:
//  AbstractPointsCommand(const QString& label,
//                        Path& path,
//                        const std::vector<LocatedSegment>& added_points);
//  AbstractPointsCommand(const QString& label, Path& path, const std::vector<Range>& removed_points);
  explicit AbstractPointsCommand(const QString& label, Path& path, std::deque<OwnedLocatedSegment>&& points_to_add);
  explicit AbstractPointsCommand(const QString& label, Path& path, const std::deque<LocatedSegmentView>& points_to_remove);
  void add();
  void remove();

  [[nodiscard]] Scene& scene() const;

private:
  Path& m_path;
  std::deque<OwnedLocatedSegment> m_points_to_add;
  std::deque<LocatedSegmentView> m_points_to_remove;
};

class AddPointsCommand : public AbstractPointsCommand
{
public:
  AddPointsCommand(Path& path, std::deque<OwnedLocatedSegment>&& added_points);
  void redo() override;
  void undo() override;
};

class RemovePointsCommand : public AbstractPointsCommand
{
public:
  RemovePointsCommand(Path& path, const std::deque<LocatedSegmentView>& removed_points);
  void redo() override;
  void undo() override;
};

}  // namespace omm
