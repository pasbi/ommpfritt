#include "properties/triggerproperty.h"

namespace omm
{

std::unique_ptr<Property> TriggerProperty::clone() const
{
  return std::make_unique<TriggerProperty>(*this);
}

void TriggerProperty::trigger()
{
  // TODO execute pre and post submit hooks
  // TODO implement set-action for python
  Q_EMIT value_changed(this);
}

}  // namespace omm
