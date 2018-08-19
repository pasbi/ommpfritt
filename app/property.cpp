#include "property.h"
#include <algorithm>
#include <assert.h>

std::unordered_set<const Object*> ReferenceProperty::m_references;

Property::Property()
{
}

Property::~Property()
{

}

ReferenceProperty::ReferenceProperty()
  : TypedProperty<Object*>(nullptr)
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
    assert(is_referenced(oldReference
      ));
    m_references.erase(oldReference);
  }
  TypedProperty<Object*>::set_value(reference);
  if (reference != nullptr) {
    m_references.emplace(reference);
  }
}

IntegerProperty::IntegerProperty(const int& defaultValue)
  : TypedProperty<int>(defaultValue)
{
}

FloatProperty::FloatProperty(const double& defaultValue)
  : TypedProperty<double>(defaultValue)
{
}

StringProperty::StringProperty(const std::string& defaultValue)
  : TypedProperty<std::string>(defaultValue)
{
}

TransformationProperty::TransformationProperty(const ObjectTransformation& defaultValue)
  : TypedProperty<ObjectTransformation>(defaultValue)
{
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