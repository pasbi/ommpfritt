#include "python/propertyownerwrapper.h"

namespace omm
{

py::object PropertyOwnerWrapper::get(const std::string& key) const
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
  py::class_<PropertyOwnerWrapper>(module, wrapped_type::TYPE, py::dynamic_attr())
        .def("get", &PropertyOwnerWrapper::get)
        .def("set", &PropertyOwnerWrapper::set);
}

void PropertyOwnerWrapper::add_property_shortcuts(py::object& object, wrapped_type& property_owner)
{
  // TODO once https://github.com/pybind/pybind11/issues/1645#issue-393712659 is resolved
  // for (auto&& key : property_owner.properties().keys()) {
    // object.attr(key.c_str()) = [&property_owner, key]() {
    //   return py::cast(property_owner.property(key).variant_value());
    // };
  // }
}

}  // namespace omm
