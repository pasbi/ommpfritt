#pragma once

#include "python/objectwrapper.h"

namespace omm
{

class PathObject;

class PathWrapper : public ObjectWrapper
{
public:
  using ObjectWrapper::ObjectWrapper;
  using wrapped_type = PathObject;
  static void define_python_interface(py::object& module);
  py::object points();
};

}  // namespace omm
