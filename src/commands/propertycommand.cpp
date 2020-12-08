#include "commands/propertycommand.h"

namespace omm
{
AbstractPropertiesCommand::AbstractPropertiesCommand(const std::set<Property*>& properties)
    : Command(QObject::tr("Set ")
              + Property::get_value<QString>(properties, std::mem_fn(&Property::label))),
      m_properties(properties)
{
}

bool AbstractPropertiesCommand::mergeWith(const QUndoCommand* command)
{
  return m_properties == dynamic_cast<const AbstractPropertiesCommand*>(command)->m_properties;
}

int AbstractPropertiesCommand::id() const
{
  return PROPERTY_COMMAND_ID;
}

}  // namespace omm
