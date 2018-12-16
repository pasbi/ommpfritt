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