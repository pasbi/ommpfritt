#pragma once

#include "animation/fcurve.h"
#include "properties/floatproperty.h"

namespace omm
{

class FloatFCurve : public FCurve<FloatProperty>
{
public:
  static constexpr auto TYPE = "FloatFCurve";
  std::string type() const override { return TYPE; }
  double interpolate(int frame) const override;
  std::size_t count() const override;

  struct KeyValue {
    double value;

    int left_offset;
    double left_value;
    int right_offset;
    double right_value;
  };

protected:
  void deserialize_keyvalue(AbstractDeserializer &, std::size_t i, const Pointer &pointer) override;
  void serialize_keyvalue(AbstractSerializer &, std::size_t i, const Pointer &pointer) const override;

private:
  std::map<int, KeyValue> m_keyvalues;
};

}  // namespace omm
