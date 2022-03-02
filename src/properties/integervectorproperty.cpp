#include "properties/integervectorproperty.h"

namespace
{
constexpr auto int_high = std::numeric_limits<int>::max();
constexpr auto int_low = std::numeric_limits<int>::lowest();
}  // namespace

namespace omm
{
const Property::PropertyDetail IntegerVectorProperty::detail{
    [](const Property&, std::size_t channel) {
      assert(channel < 2);
      return std::vector{QObject::tr("x"), QObject::tr("y")}[channel];
    }};

const Vec2i IntegerVectorPropertyLimits::lower(int_low, int_low);
const Vec2i IntegerVectorPropertyLimits::upper(int_high, int_high);
const Vec2i IntegerVectorPropertyLimits::step(1, 1);

void IntegerVectorProperty::deserialize(serialization::DeserializerWorker& worker)
{
  NumericProperty::deserialize(worker);
  set(worker.sub(TypedPropertyDetail::VALUE_POINTER)->get<Vec2i>());
  if (is_user_property()) {
    set_default_value(worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->get<Vec2i>());
  }
}

void IntegerVectorProperty::serialize(serialization::SerializerWorker& worker) const
{
  NumericProperty::serialize(worker);
  worker.sub(TypedPropertyDetail::VALUE_POINTER)->set_value(value());
  if (is_user_property()) {
    worker.sub(TypedPropertyDetail::DEFAULT_VALUE_POINTER)->set_value(default_value());
  }
}

}  // namespace omm
