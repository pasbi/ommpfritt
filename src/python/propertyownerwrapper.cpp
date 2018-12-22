#include "python/propertyownerwrapper.h"

namespace omm
{

py::object PropertyOwnerWrapper::property(const std::string& key) const
{
  auto& property_owner = wrapped<AbstractPropertyOwner>();
  if (property_owner.has_property(key)) {
    return py::cast(property_owner.property(key).variant_value());
  } else {
    for (auto&& key : property_owner.properties().keys()) {
      LOG(INFO) << key;
    }
    return py::none();
  }
}

bool PropertyOwnerWrapper::set(const std::string& key, const py::object& value) const
{
  auto& property_owner = wrapped<AbstractPropertyOwner>();
  if (property_owner.has_property(key)) {
    property_owner.property(key).set(value.cast<Property::variant_type>());
    return true;
  } else {
    return false;
  }
}

void PropertyOwnerWrapper::define_python_interface(py::object& module)
{
  py::class_<PropertyOwnerWrapper>(module, wrapped_type::TYPE)
        .def("property", &PropertyOwnerWrapper::property)
        .def("set", &PropertyOwnerWrapper::set);
}

}  // namespace omm
