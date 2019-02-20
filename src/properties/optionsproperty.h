#pragma once

#include <vector>
#include "properties/typedproperty.h"
#include <Qt>

namespace omm
{

class OptionsProperty : public TypedProperty<size_t>
{
public:
  using TypedProperty::TypedProperty;
  std::string type() const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void set(const variant_type& variant) override;
  static constexpr auto TYPE = QT_TR_NOOP("OptionsProperty");
  std::unique_ptr<Property> clone() const override;

  std::vector<std::string> options() const;
  OptionsProperty& set_options(const std::vector<std::string>& options);

  static constexpr auto OPTIONS_POINTER = "options";
  bool is_compatible(const Property& other) const override;
  void revise() override;

private:
  std::vector<std::string> m_options;
};

}  // namespace omm
