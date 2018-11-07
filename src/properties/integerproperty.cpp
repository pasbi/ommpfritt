#include "properties/integerproperty.h"

namespace omm
{

std::string IntegerProperty::type() const
{
  return "IntegerProperty";
}

std::string IntegerProperty::widget_type() const
{
  return "IntegerPropertyWidget";
}

template class TypedProperty<int>;

}  // namespace omm
