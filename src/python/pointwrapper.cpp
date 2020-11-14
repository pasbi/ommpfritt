#include "python/pointwrapper.h"
#include "geometry/vec2.h"
#include <pybind11/stl.h>

namespace omm
{
void PointWrapper::define_python_interface(py::object& module)
{
  py::class_<PointWrapper>(module, wrapped_type::TYPE)
      .def("position", &PointWrapper::position)
      .def("left_tangent", &PointWrapper::left_tangent)
      .def("right_tangent", &PointWrapper::right_tangent)
      .def("set_position", &PointWrapper::set_position)
      .def("set_left_tangent", &PointWrapper::set_left_tangent)
      .def("set_right_tangent", &PointWrapper::set_right_tangent);
}

py::object PointWrapper::left_tangent() const
{
  return py::cast(wrapped.left_tangent.to_cartesian().to_stdvec());
}

py::object PointWrapper::right_tangent() const
{
  return py::cast(wrapped.right_tangent.to_cartesian().to_stdvec());
}

py::object PointWrapper::position() const
{
  return py::cast(wrapped.position.to_stdvec());
}

void PointWrapper::set_left_tangent(const py::object& value)
{
  wrapped.left_tangent = PolarCoordinates(Vec2f(value.cast<std::vector<double>>()));
}

void PointWrapper::set_right_tangent(const py::object& value)
{
  wrapped.right_tangent = PolarCoordinates(Vec2f(value.cast<std::vector<double>>()));
}

void PointWrapper::set_position(const py::object& value)
{
  wrapped.position = Vec2f(value.cast<std::vector<double>>());
}

}  // namespace omm
