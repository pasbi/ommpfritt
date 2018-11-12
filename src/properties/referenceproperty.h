#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class HasProperties;
class ReferenceProperty : public TypedProperty<HasProperties*>
{
public:
  using ReferenceType = HasProperties*;
  /**
   * @brief creates a ReferenceProperty with no (aka nullptr) reference
   * @details the constructor does not take an argument because the default reference is nullptr
   *  always.
   */
  ReferenceProperty();
  static bool is_referenced(const ReferenceType& candidate);
  void set_value(const ReferenceType& value) override;
  std::string type() const override;
  std::string widget_type() const override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

private:
  // default is always nullptr
  void set_default_value(const ReferenceType& value) override;

private:
  static std::set<ReferenceType> m_references;
};

}  // namespace omm
