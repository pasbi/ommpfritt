#include "properties/floatproperty.h"

namespace omm
{

std::string FloatProperty::type() const
{
  return "FloatProperty";
}

std::string FloatProperty::widget_type() const
{
  return "FloatPropertyWidget";
}

}  // namespace omm
