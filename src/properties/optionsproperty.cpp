#include "properties/optionsproperty.h"
#include <algorithm>

namespace omm
{

void OptionsProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_size_t(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(
    deserializer.get_size_t(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));

  if (m_options.empty()) {
    // if options are already there, don't overwrite them because they are probably already
    //  translated.
    const std::size_t n_options = deserializer.array_size(make_pointer(root, OPTIONS_POINTER));
    m_options.reserve(n_options);
    for (std::size_t i = 0; i < n_options; ++i) {
      const auto option = deserializer.get_string(make_pointer(root, OPTIONS_POINTER, i));
      m_options.push_back(option);
    }
  }

}

void OptionsProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
  serializer.start_array(m_options.size(), OPTIONS_POINTER);
  for (std::size_t i = 0; i < m_options.size(); ++i) {
    serializer.set_value(m_options[i], make_pointer(root, OPTIONS_POINTER, i));
  }
  serializer.end_array();
}

void OptionsProperty::set(const variant_type& variant)
{
  if (std::holds_alternative<int>(variant)) {
    TypedProperty<size_t>::set(std::get<int>(variant));
  } else {
    TypedProperty<size_t>::set(variant);
  }
}

std::unique_ptr<Property> OptionsProperty::clone() const
{
  return std::make_unique<OptionsProperty>(*this);
}

std::vector<std::string> OptionsProperty::options() const { return m_options; }

OptionsProperty& OptionsProperty::set_options(const std::vector<std::string>& options)
{
  m_options = options;
  assert(m_options.size() > 0);
  set(std::clamp(value(), std::size_t(0), m_options.size()));
  return *this;
}

bool OptionsProperty::is_compatible(const Property& other) const
{
  if (TypedProperty::is_compatible(other)) {
    const auto& options_property = static_cast<const OptionsProperty&>(other);
    return options_property.m_options == m_options;
  } else {
    return false;
  }
}

void OptionsProperty::revise()
{
  set(std::clamp<std::size_t>(0, this->value(), m_options.size() - 1));
}

}  // namespace omm
