#pragma once

#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{
class StringProperty : public TypedProperty<QString>
{
public:
  explicit StringProperty(const QString& default_value = "");
  enum class Mode { SingleLine, MultiLine, FilePath, Code, Font };
  void deserialize(serialization::DeserializerWorker& worker) override;
  void serialize(serialization::SerializerWorker& worker) const override;
  StringProperty& set_mode(Mode mode);
  static constexpr auto MODE_PROPERTY_KEY = "mode";

  static const PropertyDetail detail;
};

}  // namespace omm
