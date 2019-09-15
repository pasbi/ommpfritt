#include "properties/stringproperty.h"

namespace omm
{

StringProperty::StringProperty(const std::string& default_value)
  : TypedProperty<std::string>(default_value)
{
  set_mode(Mode::SingleLine);
}

void StringProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_string(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(deserializer.get_string(
    make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  const auto mode_pointer = make_pointer(root, StringProperty::MODE_PROPERTY_KEY);
  configuration[MODE_PROPERTY_KEY] = deserializer.get_size_t(mode_pointer);
}

void StringProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
  const auto mode_pointer = make_pointer(root, StringProperty::MODE_PROPERTY_KEY);

  const Mode mode = configuration.get<Mode>(MODE_PROPERTY_KEY);
  serializer.set_value(static_cast<std::size_t>(static_cast<std::size_t>(mode)), mode_pointer);
}

StringProperty &StringProperty::set_mode(StringProperty::Mode mode)
{
  configuration[MODE_PROPERTY_KEY] = static_cast<std::size_t>(mode);
  return *this;
}

std::unique_ptr<Property> StringProperty::clone() const
{
  return std::make_unique<StringProperty>(*this);
}


}  // namespace omm
