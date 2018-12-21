#include "python/propertyownerwrapper.h"

namespace omm
{

py::object PropertyOwnerWrapper::property(const std::string& key) const
{
  return py::cast(wrapped<AbstractPropertyOwner>().property(key).variant_value());
}

void PropertyOwnerWrapper::set(const std::string& key, const py::object& value) const
{
  wrapped<AbstractPropertyOwner>().property(key).set(value.cast<Property::variant_type>());
}

void PropertyOwnerWrapper::define_python_interface(py::object& module)
{
  py::class_<PropertyOwnerWrapper>(module, wrapped_type::TYPE)
        .def("property", &PropertyOwnerWrapper::property)
        .def("set", &PropertyOwnerWrapper::set);
}

}  // namespace omm
