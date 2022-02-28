#include "properties/optionproperty.h"
#include <algorithm>

namespace omm
{
const Property::PropertyDetail OptionProperty::detail{nullptr};

void OptionProperty::deserialize(serialization::DeserializerWorker& worker)
{
  TypedProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get_size_t());

  if (is_user_property()) {
    set_default_value(worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->get_size_t());

    auto options = this->options();
    if (options.empty()) {
      // if options are already there, don't overwrite them because they are probably already
      //  translated.
      worker.sub(OPTIONS_POINTER)->get_items([&options](auto& worker_i) {
        options.push_back(worker_i.get_string());
      });
      configuration.set(OPTIONS_POINTER, options);
    }
  }
}

void OptionProperty::serialize(serialization::SerializerWorker& worker) const
{
  TypedProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  if (is_user_property()) {
    worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->set_value(default_value());
    worker.sub(OPTIONS_POINTER)->set_value(options(), [](const auto& option, auto& worker_i) {
      worker_i.set_value(option);
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
