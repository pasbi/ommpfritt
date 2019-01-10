#pragma once

#include <list>
#include "commands/command.h"
#include "geometry/point.h"
#include "objects/path.h"

namespace omm
{

class Path;
class ModifyTangentsCommand : public Command
{
public:
  class PointWithAlternative
  {
  public:
    PointWithAlternative(Point& point, const Point& alternative);
    void swap();
    bool operator==(const PointWithAlternative& other) const;
  private:
    Point& m_point;
    Point m_alternative;
  };

  ModifyTangentsCommand(Path* path, const std::list<PointWithAlternative>& alternatives);
  void undo() override;
  void redo() override;
  int id() const override;
  bool mergeWith(const QUndoCommand* command) override;

private:
  Path* m_path;
  std::list<PointWithAlternative> m_alternatives;
  Path::InterpolationMode m_old_interpolation_mode;
  void swap();
};

}  // namespace
