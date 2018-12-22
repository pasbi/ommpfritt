#pragma once

#include "python/pywrapper.h"

namespace omm
{

class PropertyOwnerWrapper : public PyWrapper
{
public:
  using PyWrapper::PyWrapper;
  py::object property(const std::string& key) const;
  bool set(const std::string& key, const py::object& value) const;
  static void define_python_interface(py::object& module);
  using wrapped_type = AbstractPropertyOwner;
};

}  // namespace omm
