#include "properties/triggerproperty.h"

namespace omm
{
const Property::PropertyDetail TriggerProperty::detail{nullptr};

void TriggerProperty::trigger()
{
  // TODO execute pre and post submit hooks
  // TODO implement set-action for python
  Q_EMIT value_changed(this);
}

}  // namespace omm
