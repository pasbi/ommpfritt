#include "properties/property.h"
#include <algorithm>
#include <assert.h>
#include "objects/object.h"

namespace omm
{

nlohmann::json TransformationProperty::to_json() const
{
  return value().to_json();
}

std::unordered_set<const Object*> ReferenceProperty::m_references;

Property::Property(const std::string& label, const std::string& category)
  : m_label(label)
  , m_category(category)
{
}

Property::~Property()
{

}

std::string Property::label() const
{
  return m_label;
}

std::string Property::category() const
{
  return m_category;
}

ReferenceProperty::ReferenceProperty(const std::string& label, const std::string& category)
  : TypedProperty<Object*>(label, category, nullptr)
{
}

bool ReferenceProperty::is_referenced(const Object* candidate)
{
  return std::find(m_references.begin(), m_references.end(), candidate) != m_references.end();
}

void ReferenceProperty::set_value(Object* reference)
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

nlohmann::json ReferenceProperty::to_json() const
{
  return value() ? nlohmann::json(value()->id()) : nlohmann::json();
}

template<>
py::object TypedProperty<Object*>::get_py_object() const
{
  return py::object();
}

template<>
void TypedProperty<Object*>::set_py_object(const py::object& value)
{
}

}  // namespace omm
