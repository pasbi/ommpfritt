#include "properties/referenceproperty.h"
#include "objects/object.h"

namespace omm
{

ReferenceProperty::ReferenceProperty()
  : TypedProperty(nullptr)
{
  set_default_value(nullptr);
}

std::string ReferenceProperty::type() const
{
  return "ReferenceProperty";
}

std::string ReferenceProperty::widget_type() const
{
  return "ReferencePropertyWidget";
}

void ReferenceProperty::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  TypedProperty::serialize(serializer, root);
  serializer.set_value(value(), make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
}

void ReferenceProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);

  // not all objects are restored yet, hence not all pointers are known.
  // remember the property to set `m_value` later.
  const auto id = deserializer.get_size_t(make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  deserializer.register_reference_property(*this, id);
}

void ReferenceProperty::set_default_value(const value_type& value)
{
  assert(value == nullptr);
  TypedProperty::set_default_value(value);
}

bool ReferenceProperty::is_allowed(AbstractPropertyOwner::Kind candidate) const
{
  return !!(allowed_kinds() & candidate);
}

AbstractPropertyOwner::Kind ReferenceProperty::allowed_kinds() const
{
  return m_allowed_kinds;
}

void ReferenceProperty::set_allowed_kinds(AbstractPropertyOwner::Kind allowed_kinds)
{
  m_allowed_kinds = allowed_kinds;
}

bool ReferenceProperty::is_compatible(const Property& other) const
{
  if (Property::is_compatible(other)) {
    auto other_reference_property = static_cast<const ReferenceProperty&>(other);
    return other_reference_property.allowed_kinds() == allowed_kinds();
  } else {
    return false;
  }
}

}   // namespace omm
