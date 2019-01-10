#include "commands/modifytangentscommand.h"

namespace omm
{

ModifyTangentsCommand::PointWithAlternative
::PointWithAlternative(Point& point, const Point& alternative)
  : m_point(point), m_alternative(alternative)
{}

void ModifyTangentsCommand::PointWithAlternative::swap()
{
  m_point.swap(m_alternative);
}

bool ModifyTangentsCommand::PointWithAlternative
::operator==(const PointWithAlternative& other) const
{
  return &m_point == &other.m_point;
}

ModifyTangentsCommand
::ModifyTangentsCommand(Path* path, const std::list<PointWithAlternative>& alternatives)
  : Command("ModifyTangentsCommand")
  , m_alternatives(alternatives)
  , m_path(path)
{

}

void ModifyTangentsCommand::undo()
{
  swap();
  if (m_path != nullptr) { m_path->set_interpolation_mode(m_old_interpolation_mode); }
}

void ModifyTangentsCommand::redo()
{
  swap();
  if (m_path != nullptr) { m_path->set_interpolation_mode(Path::InterpolationMode::Bezier); }
}

void ModifyTangentsCommand::swap()
{
  for (auto& point_with_alternative : m_alternatives) {
    point_with_alternative.swap();
  }
}

int ModifyTangentsCommand::id() const
{
  return Command::MODIFY_TANGENTS_COMMAND_ID;
}

bool ModifyTangentsCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& mtc = static_cast<const ModifyTangentsCommand&>(*command);
  return mtc.m_alternatives == m_alternatives;
}

}  // namespace omm
