#pragma once

#include "objects/path.h"
#include "python/objectwrapper.h"

namespace omm
{
class PathWrapper : public ObjectWrapper
{
public:
  using ObjectWrapper::ObjectWrapper;
  using wrapped_type = Path;
  static void define_python_interface(py::object& module);
  py::object points();
};

}  // namespace omm
