#pragma once

#include "path/pathpoint.h"
#include "python/pywrapper.h"

namespace omm
{

class PathPointWrapper : public PyWrapper<PathPoint>
{
public:
  using PyWrapper::PyWrapper;
  static void define_python_interface(py::object& module);
  [[nodiscard]] py::object geometry() const;
  void set_geometry(const py::object& value);
};

}  // namespace omm
