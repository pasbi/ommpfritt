#include "properties/floatvectorproperty.h"

namespace
{
constexpr auto inf = std::numeric_limits<double>::infinity();
}

namespace omm
{
const Property::PropertyDetail FloatVectorProperty::detail{
    [](const Property&, std::size_t channel) {
      assert(channel < 2);
      return std::vector{QObject::tr("x"), QObject::tr("y")}[channel];
    }};

const Vec2f FloatVectorPropertyLimits::lower(-inf, -inf);
const Vec2f FloatVectorPropertyLimits::upper(inf, inf);
const Vec2f FloatVectorPropertyLimits::step(1.0, 1.0);

void FloatVectorProperty::deserialize(serialization::DeserializerWorker& worker)
{
  NumericProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get<Vec2f>());
  if (is_user_property()) {
    set_default_value(worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->get<Vec2f>());
  }
}

void FloatVectorProperty::serialize(serialization::SerializerWorker& worker) const
{
  NumericProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  if (is_user_property()) {
    worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->set_value(default_value());
  }
}

}  // namespace omm
