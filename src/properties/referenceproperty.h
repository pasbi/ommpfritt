#pragma once

#include "properties/typedproperty.h"
#include "aspects/propertyowner.h"

namespace omm
{

class AbstractPropertyOwner;

class ReferenceProperty : public TypedProperty<AbstractPropertyOwner*>
{
public:
  /**
   * @brief creates a ReferenceProperty with no (aka nullptr) reference
   * @details the constructor does not take an argument because the default reference is nullptr
   *  always.
   */
  ReferenceProperty();
  std::string type() const override;
  std::string widget_type() const override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  bool is_allowed(AbstractPropertyOwner::Kind candidate) const;
  AbstractPropertyOwner::Kind allowed_kinds() const;
  void set_allowed_kinds(AbstractPropertyOwner::Kind allowed_kinds);
  bool is_compatible(const Property& other) const override;

private:
  // default is always nullptr
  void set_default_value(const value_type& value) override;
  AbstractPropertyOwner::Kind m_allowed_kinds;
};

}  // namespace omm
