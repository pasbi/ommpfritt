#include "python/pointwrapper.h"
#include <armadillo>
#include <pybind11/stl.h>
#include "common.h"

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
  return py::cast(arma::conv_to<std::vector<double>>::from(wrapped().left_tangent));
}

py::object PointWrapper::right_tangent() const
{
  return py::cast(arma::conv_to<std::vector<double>>::from(wrapped().right_tangent));
}

py::object PointWrapper::position() const
{
  return py::cast(arma::conv_to<std::vector<double>>::from(wrapped().position));
}

void PointWrapper::set_left_tangent(const py::object& value)
{
   wrapped().left_tangent = to_vec2(value);
}

void PointWrapper::set_right_tangent(const py::object& value)
{
  wrapped().right_tangent = to_vec2(value);
}

void PointWrapper::set_position(const py::object& value)
{
  wrapped().position = to_vec2(value);
}


}  // namespace omm
