#include "properties/referenceproperty.h"
#include "objects/object.h"

namespace omm
{

std::unordered_set<const Object*> ReferenceProperty::m_references;

ReferenceProperty::ReferenceProperty()
  : TypedProperty(nullptr)
{
}

bool ReferenceProperty::is_referenced(const Object* candidate)
{
  return std::find(m_references.begin(), m_references.end(), candidate) != m_references.end();
}

void ReferenceProperty::set_value(Object* const& reference)
{
  const Object* oldReference = value();
  if (oldReference != nullptr) {
    assert(is_referenced(oldReference));
    m_references.erase(oldReference);
  }
  TypedProperty<Object*>::set_value(reference);
  if (reference != nullptr) {
    m_references.emplace(reference);
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

void ReferenceProperty::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Property::deserialize(deserializer, root);
  set_value(deserializer.get_object_reference(
    make_pointer(root, TypedPropertyDetail::VALUE_POINTER)));
  set_default_value(deserializer.get_object_reference(
    make_pointer(root, TypedPropertyDetail::DEFAULT_VALUE_POINTER)));
}

}   // namespace omm
