#pragma once

#include "properties/typedproperty.h"

namespace omm
{

class ReferenceProperty : public TypedProperty<Serializable*>
{
public:
  using ReferenceType = Serializable*;
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
