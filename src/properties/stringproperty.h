#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class StringProperty : public TypedProperty<std::string>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = "StringProperty";
  bool is_multi_line() const;
  StringProperty& set_is_multi_line(bool is_multiline);
  static constexpr auto IS_MULTILINE_KEY = "is_multiline";
  std::unique_ptr<Property> clone() const override;

private:
  bool m_is_multi_line = false;
};

}  // namespace omm
