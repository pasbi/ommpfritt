#pragma once

#include "properties/typedproperty.h"
#include <Qt>
#include <vector>

namespace omm
{
class OptionProperty : public TypedProperty<std::size_t>
{
public:
  using TypedProperty::TypedProperty;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void set(const variant_type& variant) override;

  [[nodiscard]] std::vector<QString> options() const;
  OptionProperty& set_options(const std::vector<QString>& options);

  static constexpr auto OPTIONS_POINTER = "options";
  [[nodiscard]] bool is_compatible(const Property& other) const override;
  void revise() override;
  static const PropertyDetail detail;
};

}  // namespace omm
