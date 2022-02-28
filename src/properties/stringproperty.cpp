#include "properties/stringproperty.h"

namespace omm
{
const Property::PropertyDetail StringProperty::detail{nullptr};

StringProperty::StringProperty(const QString& default_value) : TypedProperty<QString>(default_value)
{
  set_mode(Mode::SingleLine);
}

void StringProperty::deserialize(serialization::DeserializerWorker& worker)
{
  TypedProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get_string());
  if (is_user_property()) {
    set_default_value(worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->get_string());
    configuration.set(MODE_PROPERTY_KEY, worker.sub(StringProperty::MODE_PROPERTY_KEY)->get_size_t());
  }
}

void StringProperty::serialize(serialization::SerializerWorker& worker) const
{
  TypedProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  if (is_user_property()) {
    worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->set_value(default_value());
    const Mode mode = configuration.get<Mode>(MODE_PROPERTY_KEY);
    worker.sub(StringProperty::MODE_PROPERTY_KEY)->set_value(static_cast<std::size_t>(static_cast<std::size_t>(mode)));
  }
}

StringProperty& StringProperty::set_mode(StringProperty::Mode mode)
{
  configuration.set(MODE_PROPERTY_KEY, mode);
  Q_EMIT this->configuration_changed();
  return *this;
}

}  // namespace omm
