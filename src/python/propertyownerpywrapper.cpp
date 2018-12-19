#include "python/propertyownerpywrapper.h"
#include <pybind11/stl.h>
#include "aspects/propertyowner.h"

namespace omm
{

PropertyOwnerPyWrapper::PropertyOwnerPyWrapper(AbstractPropertyOwner& wrapped)
  : m_wrapped(&wrapped)
{
}

py::object PropertyOwnerPyWrapper::get_property(const std::string& key) const
{
  if (m_wrapped->has_property(key)) {
    return py::cast(m_wrapped->property(key).variant_value());
  } else {
    return py::none();
  }
}

void PropertyOwnerPyWrapper::set_property(const std::string& key, const py::object& value) const
{
  if (m_wrapped->has_property(key)) {
    m_wrapped->property(key).set(value.cast<Property::variant_type>());
  } else {
    throw pybind11::key_error(key);
  }
}

bool operator<(const PropertyOwnerPyWrapper& a, const PropertyOwnerPyWrapper& b)
{
  return static_cast<void*>(a.m_wrapped) < static_cast<void*>(b.m_wrapped);
}

}  // namespace omm
