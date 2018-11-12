#include "commands/propertycommand.h"

namespace omm
{

AbstractPropertiesCommand::AbstractPropertiesCommand(const Property::SetOfProperties& properties)
  : Command(QObject::tr("Set ").toStdString() + Property::get_label(properties))
  , m_properties(properties)
{
}

bool AbstractPropertiesCommand::mergeWith(const QUndoCommand* command)
{
  const auto& properties_command = static_cast<const AbstractPropertiesCommand&>(*command);
  return properties_command.m_properties == m_properties;
}

int AbstractPropertiesCommand::id() const
{
  return PROPERTY_COMMAND_ID;
}

}  // namespace omm