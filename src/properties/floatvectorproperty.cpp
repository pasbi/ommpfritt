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

void FloatVectorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  NumericProperty::deserialize(deserializer, root);
  set(deserializer.get_vec2f(make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  if (is_user_property()) {
    const auto ptr = make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER);
    set_default_value(deserializer.get_vec2f(ptr));
  }
}

void FloatVectorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  NumericProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  if (is_user_property()) {
    serializer.set_value(default_value(),
                         make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER));
  }
}

}  // namespace omm
