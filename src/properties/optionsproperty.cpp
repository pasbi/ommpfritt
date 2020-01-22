#include "properties/optionsproperty.h"
#include <algorithm>

namespace omm
{

const Property::PropertyDetail OptionsProperty::detail { nullptr };

void OptionsProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);
  set(deserializer.get_size_t(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));

  if (is_user_property()) {
    set_default_value(
      deserializer.get_size_t(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));

    auto options = this->options();
    if (options.empty()) {
      // if options are already there, don't overwrite them because they are probably already
      //  translated.
      const std::size_t n_options = deserializer.array_size(make_pointer(root, OPTIONS_POINTER));
      options.reserve(n_options);
      for (std::size_t i = 0; i < n_options; ++i) {
        const auto option = deserializer.get_string(make_pointer(root, OPTIONS_POINTER, i));
        options.push_back(option);
      }
      configuration[OPTIONS_POINTER] = options;
    }
  }
}

void OptionsProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value( default_value(),
                          make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
    const auto options = this->options();
    serializer.start_array(options.size(), make_pointer(root, OPTIONS_POINTER));
    for (std::size_t i = 0; i < options.size(); ++i) {
      serializer.set_value(options[i], make_pointer(root, OPTIONS_POINTER, i));
    }
    serializer.end_array();
  }
}

void OptionsProperty::set(const variant_type& variant)
{
  if (std::holds_alternative<int>(variant)) {
    TypedProperty<size_t>::set(std::get<int>(variant));
  } else {
    assert(std::holds_alternative<std::size_t>(variant));
    TypedProperty<size_t>::set(variant);
  }
}

std::unique_ptr<Property> OptionsProperty::clone() const
{
  return std::make_unique<OptionsProperty>(*this);
}

std::vector<QString> OptionsProperty::options() const
{
  if (configuration.count(OPTIONS_POINTER) > 0) {
    return configuration.get<std::vector<QString>>(OPTIONS_POINTER);
  } else {
    return std::vector<QString>();
  }
}

OptionsProperty& OptionsProperty::set_options(const std::vector<QString>& options)
{
  configuration[OPTIONS_POINTER] = options;
  assert(options.size() > 0);
  set(std::clamp(value(), std::size_t(0), options.size()));
  return *this;
}

bool OptionsProperty::is_compatible(const Property& other) const
{
  if (TypedProperty::is_compatible(other)) {
    const auto& other_op = static_cast<const OptionsProperty&>(other);
    return options() == other_op.options();
  } else {
    return false;
  }
}

void OptionsProperty::revise()
{
  set(std::clamp<std::size_t>(0, this->value(), options().size() - 1));
}

}  // namespace omm
