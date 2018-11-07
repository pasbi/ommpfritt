#include "properties/stringproperty.h"

namespace omm
{

std::string StringProperty::type() const
{
  return "StringProperty";
}

std::string StringProperty::widget_type() const
{
  return "StringPropertyWidget";
}

}  // namespace omm
