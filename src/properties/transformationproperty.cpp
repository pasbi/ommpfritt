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

void TransformationProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Property::deserialize(deserializer, root);
  set_value(deserializer.get_object_transformation(
    make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(deserializer.get_object_transformation(
    make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)) );
}

}  // namespace omm
