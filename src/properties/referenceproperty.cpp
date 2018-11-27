#include "properties/referenceproperty.h"
#include "objects/object.h"

namespace omm
{

std::set<ReferenceProperty::ReferenceType> ReferenceProperty::m_references;

ReferenceProperty::ReferenceProperty()
  : TypedProperty(nullptr)
{
  set_default_value(nullptr);
}

bool ReferenceProperty::is_referenced(const ReferenceType& candidate)
{
  return std::find(m_references.begin(), m_references.end(), candidate) != m_references.end();
}

void ReferenceProperty::set_value(const ReferenceType& reference)
{
  if (value() != reference) {
    const auto old_reference = value();
    if (old_reference != nullptr) {
      assert(is_referenced(old_reference));
      m_references.erase(old_reference);
    }
    TypedProperty::set_value(reference);
    if (reference != nullptr) {
      m_references.emplace(reference);
    }
  }
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
  const auto id = value() == nullptr ? 0 : value()->identify();
  serializer.set_value(id, make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
}

void ReferenceProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  TypedProperty::deserialize(deserializer, root);

  // not all objects are restored yet, hence not all pointers are known.
  // remember the property to set `m_value` later.
  const auto id = deserializer.get_id(make_pointer(root, TypedPropertyDetail::VALUE_POINTER));
  deserializer.register_reference_property(*this, id);
}

void ReferenceProperty::set_default_value(const ReferenceType& value)
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

}   // namespace omm
