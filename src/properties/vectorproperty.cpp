#include "properties/vectorproperty.h"

namespace omm
{

std::string FloatVectorProperty::type() const { return TYPE; }

void FloatVectorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_double(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_vec2(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void FloatVectorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

std::unique_ptr<Property> FloatVectorProperty::clone() const
{
  return std::make_unique<FloatVectorProperty>(*this);
}

std::string IntegerVectorProperty::type() const { return TYPE; }

void IntegerVectorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_double(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_ivec2(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void IntegerVectorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

std::unique_ptr<Property> IntegerVectorProperty::clone() const
{
  return std::make_unique<IntegerVectorProperty>(*this);
}

}  // namespace omm
