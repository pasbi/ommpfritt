#include "properties/stringproperty.h"

namespace omm
{

std::string StringProperty::type() const
{
  return TYPE;
}

bool StringProperty::is_multi_line() const
{
  return m_is_multi_line;
}

StringProperty& StringProperty::set_is_multi_line(bool is_multiline)
{
  m_is_multi_line = is_multiline;
  return *this;
}

void StringProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_string(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(deserializer.get_string(
    make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  m_is_multi_line = deserializer.get_bool(make_pointer(root, StringProperty::IS_MULTILINE_KEY));
}

void StringProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
  serializer.set_value(m_is_multi_line, make_pointer(root, StringProperty::IS_MULTILINE_KEY));
}

std::unique_ptr<Property> StringProperty::clone() const
{
  return std::make_unique<StringProperty>(*this);
}

}  // namespace omm
