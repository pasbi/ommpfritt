#pragma once

#include "commands/command.h"
#include "geometry/point.h"
#include "objects/path.h"
#include <list>

namespace omm
{
class Path;
class ModifyPointsCommand : public Command
{
public:
  using map_type = std::map<Path::iterator, Point>;
  ModifyPointsCommand(const map_type& points);
  void redo() override;
  void undo() override;
  int id() const override;
  bool mergeWith(const QUndoCommand* command) override;

private:
  std::map<Path::iterator, Point> m_data;
  void swap();
};

class AbstractPointsCommand : public Command
{
public:
  struct LocatedSegment {
    Path::iterator index;
    Path::Segment points;
    bool operator<(const LocatedSegment& other) const;
    bool operator>(const LocatedSegment& other) const;
  };

  struct Range {
    Path::iterator begin;
    std::size_t length;
    bool intersects(const Range& other) const;
    bool operator<(const Range& other) const;
    bool operator>(const Range& other) const;
  };

protected:
  AbstractPointsCommand(const QString& label,
                        Path& path,
                        const std::vector<LocatedSegment>& added_points);
  AbstractPointsCommand(const QString& label, Path& path, const std::vector<Range>& removed_points);
  void add();
  void remove();

  Scene& scene() const;

private:
  Path& m_path;
  std::vector<LocatedSegment> m_points_to_add;
  std::vector<Range> m_points_to_remove;
};

class AddPointsCommand : public AbstractPointsCommand
{
public:
  AddPointsCommand(Path& path, const std::vector<LocatedSegment>& added_points);
  void redo() override;
  void undo() override;
};

class RemovePointsCommand : public AbstractPointsCommand
{
public:
  RemovePointsCommand(Path& path, const std::vector<Range>& removed_points);
  void redo() override;
  void undo() override;
};

}  // namespace omm
