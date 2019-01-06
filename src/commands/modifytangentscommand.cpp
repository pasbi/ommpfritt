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

ModifyTangentsCommand::ModifyTangentsCommand(const std::vector<PointWithAlternative>& alternatives)
  : Command("ModifyTangentsCommand")
  , m_alternatives(alternatives)
{

}

void ModifyTangentsCommand::undo()
{
  for (PointWithAlternative& point_with_alternative : m_alternatives) {
    point_with_alternative.swap();
  }
}

void ModifyTangentsCommand::redo()
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
