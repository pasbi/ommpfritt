#include "properties/optionproperty.h"
#include <algorithm>

namespace omm
{
const Property::PropertyDetail OptionProperty::detail{nullptr};

void OptionProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
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
      deserializer.get_items(make_pointer(root, OPTIONS_POINTER), [&deserializer, &options](const auto& root) {
        const auto option = deserializer.get_string(root);
        options.push_back(option);
      });
      configuration.set(OPTIONS_POINTER, options);
    }
  }
}

void OptionProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value(default_value(),
                         make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER));
    const auto options = this->options();
    const auto ptr = make_pointer(root, OPTIONS_POINTER);
    serializer.set_value(options, ptr, [&serializer](const auto& option, const auto& root) {
      serializer.set_value(option, root);
    });
  }
}

void OptionProperty::set(const variant_type& variant)
{
  if (std::holds_alternative<int>(variant)) {
    TypedProperty<std::size_t>::set(std::get<int>(variant));
  } else {
    assert(std::holds_alternative<std::size_t>(variant));
    TypedProperty<std::size_t>::set(variant);
  }
}

std::deque<QString> OptionProperty::options() const
{
  if (configuration.count(OPTIONS_POINTER) > 0) {
    return configuration.get<std::deque<QString>>(OPTIONS_POINTER);
  } else {
    return {};
  }
}

OptionProperty& OptionProperty::set_options(const std::deque<QString>& options)
{
  configuration.set(OPTIONS_POINTER, options);
  assert(!options.empty());
  set(std::clamp(value(), std::size_t(0), options.size()));
  Q_EMIT this->configuration_changed();
  return *this;
}

bool OptionProperty::is_compatible(const Property& other) const
{
  if (TypedProperty::is_compatible(other)) {
    const auto& other_op = dynamic_cast<const OptionProperty&>(other);
    return options() == other_op.options();
  } else {
    return false;
  }
}

void OptionProperty::revise()
{
  set(std::clamp<std::size_t>(0, this->value(), options().size() - 1));
}

}  // namespace omm
