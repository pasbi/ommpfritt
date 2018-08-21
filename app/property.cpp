#include "property.h"
#include <algorithm>
#include <assert.h>
#include "object.h"

nlohmann::json omm::TransformationProperty::to_json() const
{
  return value().to_json();
}

std::unordered_set<const omm::Object*> omm::ReferenceProperty::m_references;

omm::Property::Property()
{
}

omm::Property::~Property()
{

}

omm::ReferenceProperty::ReferenceProperty()
  : TypedProperty<Object*>(nullptr)
{
}

bool omm::ReferenceProperty::is_referenced(const omm::Object* candidate)
{
  return std::find(m_references.begin(), m_references.end(), candidate) != m_references.end();
}

void omm::ReferenceProperty::set_value(omm::Object* reference)
{
  const Object* oldReference = value();
  if (oldReference != nullptr) {
    assert(is_referenced(oldReference
      ));
    m_references.erase(oldReference);
  }
  TypedProperty<Object*>::set_value(reference);
  if (reference != nullptr) {
    m_references.emplace(reference);
  }
}

nlohmann::json omm::ReferenceProperty::to_json() const
{
  return value() ? nlohmann::json(value()->id()) : nlohmann::json();
}

template<>
py::object omm::TypedProperty<omm::Object*>::get_py_object() const
{
  return py::object();
}

template<>
void omm::TypedProperty<omm::Object*>::set_py_object(const py::object& value)
{
}

