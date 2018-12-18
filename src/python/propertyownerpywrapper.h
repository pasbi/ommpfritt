#pragma once

#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include "aspects/propertyowner.h"

namespace py = pybind11;

namespace omm
{

template<typename WrappedT>
class PropertyOwnerPyWrapper
{
public:
  explicit PropertyOwnerPyWrapper(WrappedT& wrapped) : wrapped(&wrapped) {}

  py::object get_property(const std::string& key) const
  {
    return py::cast(wrapped->property(key).variant_value());
  }

  void set_property(const std::string& key, const py::object& value) const
  {
    wrapped->property(key).set(value.cast<Property::variant_type>());
  }

  WrappedT* wrapped;
};

}  // namespace omm
