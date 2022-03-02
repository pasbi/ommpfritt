#include "properties/colorproperty.h"
#include <QCoreApplication>

namespace omm
{
const Property::PropertyDetail ColorProperty::detail{
    [](const Property& property, std::size_t channel) -> QString {
      const auto color = std::get<Color>(property.variant_value());
      const auto name = Color::component_name(color.model(), channel);
      return QCoreApplication::translate("Color", name.toStdString().c_str());
    }};

void ColorProperty::deserialize(serialization::DeserializerWorker& worker)
{
  TypedProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get<Color>());
  if (is_user_property()) {
    set_default_value(worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->get<Color>());
  }
}

void ColorProperty::serialize(serialization::SerializerWorker& worker) const
{
  TypedProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  if (is_user_property()) {
    worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->set_value(default_value());
  }
}

}  // namespace omm
