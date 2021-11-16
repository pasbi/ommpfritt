#pragma once

#include "commands/command.h"
#include <deque>
#include <memory>

namespace omm
{

class Path;
class Point;
class PathPoint;
class Segment;
struct SegmentView;

class ModifyPointsCommand : public Command
{
public:
  using Map = std::map<Path*, std::map<PathPoint*, Point>>;
  ModifyPointsCommand(const Map& points);
  void redo() override;
  void undo() override;
  [[nodiscard]] int id() const override;
  bool mergeWith(const QUndoCommand* command) override;
  [[nodiscard]] bool is_noop() const override;

private:
  Map m_data;
  void exchange();
};

class AbstractPointsCommand : public Command
{
public:
  class OwnedLocatedSegment
  {
  public:
    explicit OwnedLocatedSegment(Segment* segment, std::size_t index, std::deque<std::unique_ptr<PathPoint>>&& points);
    explicit OwnedLocatedSegment(std::unique_ptr<Segment> segment);
    SegmentView insert_into(Path& path);
    friend bool operator<(const OwnedLocatedSegment& a, const OwnedLocatedSegment& b);

  private:
    Segment* m_segment = nullptr;
    std::unique_ptr<Segment> m_owned_segment{};
    std::size_t m_index;
    std::deque<std::unique_ptr<PathPoint>> m_points;
  };

protected:
  explicit AbstractPointsCommand(const QString& label, Path& path, std::deque<OwnedLocatedSegment>&& points_to_add);
  explicit AbstractPointsCommand(const QString& label, Path& path, std::deque<SegmentView>&& points_to_remove);
  void add();
  void remove();

  [[nodiscard]] Scene& scene() const;

private:
  Path& m_path;
  std::deque<OwnedLocatedSegment> m_points_to_add;
  std::deque<SegmentView> m_points_to_remove;
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
  RemovePointsCommand(Path& path, std::deque<SegmentView>&& removed_points);
  void redo() override;
  void undo() override;
};

}  // namespace omm
