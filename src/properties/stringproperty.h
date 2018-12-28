#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class StringProperty : public TypedProperty<std::string>
{
public:
  enum class LineMode { MultiLine, SingleLine };
  using TypedProperty::TypedProperty;
  explicit StringProperty(std::string defaultValue, LineMode mode);
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = "StringProperty";
  StringProperty::LineMode line_mode() const;
  static constexpr auto IS_MULTILINE_KEY = "is_multiline";
  std::unique_ptr<Property> clone() const override;

private:
  LineMode m_line_mode = LineMode::SingleLine;
};

}  // namespace omm
