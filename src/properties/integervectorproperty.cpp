#include "properties/integervectorproperty.h"

namespace
{
static constexpr auto int_high = std::numeric_limits<int>::max();
static constexpr auto int_low = std::numeric_limits<int>::lowest();
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

void IntegerVectorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  NumericProperty::deserialize(deserializer, root);
  set(deserializer.get_vec2i(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  if (is_user_property()) {
    set_default_value(
        deserializer.get_vec2i(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
  }
}

void IntegerVectorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  NumericProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value(default_value(),
                         make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER));
  }
}

}  // namespace omm
