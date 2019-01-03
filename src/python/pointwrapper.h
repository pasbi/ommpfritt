#pragma once

#include "python/pywrapper.h"
#include "geometry/point.h"

namespace omm
{

class PointWrapper : public PyWrapper<Point>
{
public:
  using PyWrapper::PyWrapper;
  using wrapped_type = Point;
  static void define_python_interface(py::object& module);
  py::object left_tangent() const;
  py::object right_tangent() const;
  py::object position() const;
  void set_left_tangent(const py::object& value);
  void set_right_tangent(const py::object& value);
  void set_position(const py::object& value);

};

}  // namespace omm
