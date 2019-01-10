#pragma once

#include "python/pywrapper.h"
#include <type_traits>


namespace omm
{

namespace detail
{

template<typename WrappedT>
py::object get_property_value(WrappedT&& wrapped, const std::string& key)
{
  if (wrapped.has_property(key)) {
    const auto value = wrapped.property(key).variant_value();
    if (std::holds_alternative<AbstractPropertyOwner*>(value)) {
      return wrap(std::get<AbstractPropertyOwner*>(value));
    } else {
      return py::cast(value);
    }
  } else {
    LOG(ERROR) << "Failed to find property key '" << key << "'.";
    for (const std::string& key : wrapped.properties().keys()) {
      LOG(INFO) << key;
    }
    return py::none();
  }
}

bool set_property_value( AbstractPropertyOwner& property_owner,
                         const std::string& key, const py::object& value );
}  // namespace detail

template<typename WrappedT, typename = void>
class AbstractPropertyOwnerWrapper : public PyWrapper<WrappedT>
{
  static_assert(std::is_base_of<AbstractPropertyOwner, std::decay_t<WrappedT>>::value);
public:
  using PyWrapper<WrappedT>::PyWrapper;
  py::object get(const std::string& key) const
  {
    return detail::get_property_value(this->wrapped, key);
  }

  bool set(const std::string& key, const py::object& value) const
  {
    return detail::set_property_value(this->wrapped, key, value);
  }

  static void define_python_interface(py::object& module)
  {
    const auto type_name = (std::string("Base") + WrappedT::TYPE).c_str();
    py::class_<AbstractPropertyOwnerWrapper<WrappedT>>(module, type_name, py::dynamic_attr())
          .def("get", &AbstractPropertyOwnerWrapper<WrappedT>::get)
          .def("set", &AbstractPropertyOwnerWrapper<WrappedT>::set);
  }
  // static void add_property_shortcuts(pybind11::object& object, wrapped_type& property_owner);
};

}  // namespace omm
