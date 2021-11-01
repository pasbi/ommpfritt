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
  class LocatedSegmentView;
  class OwnedLocatedSegment
  {
  public:
    explicit OwnedLocatedSegment(Segment* segment, std::size_t index, std::deque<std::unique_ptr<Point>>&& points);
    explicit OwnedLocatedSegment(std::unique_ptr<Segment> segment);
    LocatedSegmentView insert_into(Path& path);

  private:
    Segment* m_segment = nullptr;
    std::unique_ptr<Segment> m_owned_segment{};
    std::size_t m_index;
    std::deque<std::unique_ptr<Point>> m_points;
  };

  class LocatedSegmentView
  {
  public:
    explicit LocatedSegmentView(Segment& segment, std::size_t index, std::size_t size);
    OwnedLocatedSegment extract_from(Path& path) const;
  private:
    Segment& m_segment;
    std::size_t m_index;
    std::size_t m_size;
  };

protected:
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
  static QString static_label();
};

class RemovePointsCommand : public AbstractPointsCommand
{
public:
  RemovePointsCommand(Path& path, const std::deque<LocatedSegmentView>& removed_points);
  void redo() override;
  void undo() override;
};

}  // namespace omm
