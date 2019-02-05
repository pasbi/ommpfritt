#include "properties/vectorproperty.h"

namespace
{
  static constexpr auto inf = std::numeric_limits<double>::infinity();
  static constexpr auto int_high = std::numeric_limits<int>::max();
  static constexpr auto int_low = std::numeric_limits<int>::lowest();
}

namespace omm
{


const arma::vec2 FloatVectorPropertyLimits::lower = arma::vec2 { -inf, -inf };
const arma::vec2 FloatVectorPropertyLimits::upper = arma::vec2 {  inf,  inf };
const arma::vec2 FloatVectorPropertyLimits::step = arma::vec2 { 1.0, 1.0 };

std::string FloatVectorProperty::type() const { return TYPE; }

void FloatVectorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  NumericProperty::deserialize(deserializer, root);
  const auto v = deserializer.get_vec2(make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  set(VectorPropertyValueType<arma::vec2>(v));
  set_default_value(
    deserializer.get_vec2(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void FloatVectorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  NumericProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

std::unique_ptr<Property> FloatVectorProperty::clone() const
{
  return std::make_unique<FloatVectorProperty>(*this);
}

const arma::ivec2 IntegerVectorPropertyLimits::lower = arma::ivec2 { int_low,  int_low  };
const arma::ivec2 IntegerVectorPropertyLimits::upper = arma::ivec2 { int_high, int_high };
const arma::ivec2 IntegerVectorPropertyLimits::step = arma::ivec2 { 1, 1 };

std::string IntegerVectorProperty::type() const { return TYPE; }

void IntegerVectorProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  NumericProperty::deserialize(deserializer, root);
  const auto v = deserializer.get_ivec2(make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  set(VectorPropertyValueType<arma::ivec2>(v));
  set_default_value(
    deserializer.get_ivec2(make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

void IntegerVectorProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  NumericProperty::serialize(serializer, root);
  serializer.set_value( value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  serializer.set_value( default_value(),
                        make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER) );
}

std::unique_ptr<Property> IntegerVectorProperty::clone() const
{
  return std::make_unique<IntegerVectorProperty>(*this);
}

}  // namespace omm
