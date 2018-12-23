#include "commands/propertycommand.h"

namespace omm
{

AbstractPropertiesCommand::AbstractPropertiesCommand(const std::set<Property*>& properties)
  : Command(   QObject::tr("Set ").toStdString()
             + Property::get_value<std::string>(properties, std::mem_fn(&Property::label)) )
  , m_properties(properties)
{
}

bool AbstractPropertiesCommand::mergeWith(const QUndoCommand* command)
{
  const auto touch_same_properties = [](const auto& a, const auto& b) {
    return a.m_properties == b.m_properties;
  };
  return touch_same_properties(*this, static_cast<const AbstractPropertiesCommand&>(*command));
}

int AbstractPropertiesCommand::id() const
{
  return PROPERTY_COMMAND_ID;
}

}  // namespace omm