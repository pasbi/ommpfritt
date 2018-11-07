#include "properties/transformationproperty.h"

namespace omm
{

std::string TransformationProperty::type() const
{
  return "TransformationProperty";
}

std::string TransformationProperty::widget_type() const
{
  return "TransformationPropertyWidget";
}

}  // namespace omm
