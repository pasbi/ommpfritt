#pragma once

#include <list>
#include "commands/command.h"
#include "geometry/point.h"
#include "objects/path.h"

namespace omm
{

class Path;
class ModifyPointsCommand : public Command
{
public:
  using map_type = std::map<Path*, std::map<Point*, Point>>;
  ModifyPointsCommand(const map_type& points);
  void redo() override;
  void undo() override;
  int id() const override;
  bool mergeWith(const QUndoCommand* command) override;

private:
  std::map<Path*, std::map<Point*, Point>> m_data;
  Path::InterpolationMode m_old_interpolation_mode;
  void swap();
};

class AbstractPointsCommand : public Command
{
protected:
  AbstractPointsCommand( const std::string& label,
                         const std::map<Path*, std::vector<Path::PointSequence>>& points );
  AbstractPointsCommand( const std::string& label,
                         const std::map<Path*, std::vector<std::size_t>>& points );
  void add();
  void remove();

  Scene& scene() const;

private:
  std::map<Path*, std::vector<std::size_t>> m_removed_points;
  std::map<Path*, std::vector<Path::PointSequence>> m_added_points;

};

class AddPointsCommand : public AbstractPointsCommand
{
public:
  AddPointsCommand(const std::map<Path*, std::vector<Path::PointSequence>>& points);
  void redo() override;
  void undo() override;
};

class RemovePointsCommand : public AbstractPointsCommand
{
public:
  RemovePointsCommand(const std::map<Path*, std::vector<std::size_t>>& points);
  void redo() override;
  void undo() override;
};

}  // namespace
