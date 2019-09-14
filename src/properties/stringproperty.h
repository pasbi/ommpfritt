#pragma once

#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{

class StringProperty : public TypedProperty<std::string>
{
public:
  explicit StringProperty(const std::string &default_value = "");
  enum class Mode { SingleLine, MultiLine, FilePath, Code, Font };
  std::string type() const override { return TYPE; }
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Property", "StringProperty");
  StringProperty& set_mode(Mode mode);
  static constexpr auto MODE_PROPERTY_KEY = "mode";
  std::unique_ptr<Property> clone() const override;
};

}  // namespace omm
