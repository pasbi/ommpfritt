#pragma once

#include "properties/typedproperty.h"
#include "aspects/propertyowner.h"

namespace omm
{

class AbstractPropertyOwner;

class ReferenceProperty : public TypedProperty<AbstractPropertyOwner*>
{
public:
  ReferenceProperty(AbstractPropertyOwner::Kind allowed_kinds = AbstractPropertyOwner::Kind::All);
  std::string type() const override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void set_allowed_kinds(AbstractPropertyOwner::Kind allowed_kinds);
  AbstractPropertyOwner::Kind allowed_kinds() const;
  bool is_compatible(const Property& other) const override;
  bool is_cyclic() const;
  static constexpr auto TYPE = "ReferenceProperty";

private:
  // default is always nullptr
  void set_default_value(const value_type& value) override;
  AbstractPropertyOwner::Kind m_allowed_kinds;
};

}  // namespace omm
