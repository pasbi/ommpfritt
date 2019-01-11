#include "properties/triggerproperty.h"

namespace omm
{

std::string TriggerProperty::type() const
{
  return TYPE;
}

void TriggerProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
}

void TriggerProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
}

std::unique_ptr<Property> TriggerProperty::clone() const
{
  return std::make_unique<TriggerProperty>(*this);
}

void TriggerProperty::trigger()
{
  // TODO execute pre and post submit hooks
  // TODO implement set-action for python
  notify_observers();
}

}  // namespace omm
