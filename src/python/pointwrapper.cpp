#include "python/pointwrapper.h"
#include "external/pybind11/stl.h"
#include "geometry/vec2.h"

namespace omm
{

PointWrapper::PointWrapper(const Point& point)
  : m_point(point)
{
}

void PointWrapper::define_python_interface(py::object& module)
{
  py::class_<PointWrapper>(module, Point::TYPE)
      .def(py::init<>())
      .def("position", &PointWrapper::position)
      .def("left_tangent", &PointWrapper::left_tangent)
      .def("right_tangent", &PointWrapper::right_tangent)
      .def("set_position", &PointWrapper::set_position)
      .def("set_left_tangent", &PointWrapper::set_left_tangent)
      .def("set_right_tangent", &PointWrapper::set_right_tangent);
}

py::object PointWrapper::left_tangent() const
{
  return py::cast(m_point.left_tangent().to_cartesian().to_stdvec());
}

py::object PointWrapper::right_tangent() const
{
  return py::cast(m_point.right_tangent().to_cartesian().to_stdvec());
}

py::object PointWrapper::position() const
{
  return py::cast(m_point.position().to_stdvec());
}

void PointWrapper::set_left_tangent(const py::object& value)
{
  m_point.set_left_tangent(PolarCoordinates(Vec2f(value.cast<std::vector<double>>())));
}

void PointWrapper::set_right_tangent(const py::object& value)
{
  m_point.set_right_tangent(PolarCoordinates(Vec2f(value.cast<std::vector<double>>())));
}

void PointWrapper::set_position(const py::object& value)
{
  m_point.set_position(Vec2f(value.cast<std::vector<double>>()));
}

const Point& PointWrapper::point() const
{
  return m_point;
}

}  // namespace omm
