#include "properties/stringproperty.h"

namespace omm
{

StringProperty::StringProperty(std::string defaultValue, LineMode mode)
  : TypedProperty(defaultValue)
  , m_line_mode(mode)
{

}

std::string StringProperty::type() const
{
  return TYPE;
}

StringProperty::LineMode StringProperty::line_mode() const
{
  return m_line_mode;
}

void StringProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_string(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(deserializer.get_string(
    make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  if (deserializer.get_bool(make_pointer(root, StringProperty::IS_MULTILINE_KEY))) {
    m_line_mode = LineMode::MultiLine;
  } else {
    m_line_mode = LineMode::SingleLine;
  }
}

void StringProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
  const bool is_multiline = m_line_mode == LineMode::MultiLine;
  serializer.set_value(is_multiline, make_pointer(root, StringProperty::IS_MULTILINE_KEY));
}

std::unique_ptr<Property> StringProperty::clone() const
{
  return std::make_unique<StringProperty>(*this);
}

}  // namespace omm
