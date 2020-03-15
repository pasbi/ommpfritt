#pragma once

#include "properties/numericproperty.h"
#include <Qt>

namespace omm
{

struct FloatVectorPropertyLimits
{
  static const Vec2f lower;
  static const Vec2f upper;
  static const Vec2f step;
};

class FloatVectorProperty : public NumericProperty<Vec2f>
{
public:
  using NumericProperty::NumericProperty;
  QString type() const override { return TYPE; }
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "FloatVectorProperty");
  static const PropertyDetail detail;
};

struct IntegerVectorPropertyLimits
{
  static const Vec2i lower;
  static const Vec2i upper;
  static const Vec2i step;
};

class IntegerVectorProperty : public NumericProperty<Vec2i>
{
public:
  using NumericProperty::NumericProperty;
  QString type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("IntegerVectorProperty", "IntegerVectorProperty");
  static const PropertyDetail detail;
};

}  // namespace omm
