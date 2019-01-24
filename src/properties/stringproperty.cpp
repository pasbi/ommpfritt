#include "properties/stringproperty.h"

namespace omm
{

std::string StringProperty::type() const { return TYPE; }
StringProperty::Mode StringProperty::mode() const { return m_mode; }

StringProperty& StringProperty::set_mode(Mode mode)
{
  m_mode = mode;
  return *this;
}

void StringProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_string(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(deserializer.get_string(
    make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  const auto mode_pointer = make_pointer(root, StringProperty::MODE_PROPERTY_KEY);
  m_mode = static_cast<Mode>(deserializer.get_size_t(mode_pointer));
}

void StringProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
  const auto mode_pointer = make_pointer(root, StringProperty::MODE_PROPERTY_KEY);
  serializer.set_value(static_cast<std::size_t>(m_mode), mode_pointer);
}

std::unique_ptr<Property> StringProperty::clone() const
{
  return std::make_unique<StringProperty>(*this);
}

}  // namespace omm
