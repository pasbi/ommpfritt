#include "python/objecttransformationwrapper.h"
#include <pybind11/stl.h>
#include <armadillo>

namespace py = pybind11;

namespace
{

auto to_vec2(const py::object& o)
{
  const std::vector<double> std_vector = o.cast<std::vector<double>>();
  if (std_vector.size() == 2) {
    return std::pair(true, arma::vec2 { std_vector[0], std_vector[1] });
  } else {
    return std::pair(false, arma::vec2());
  }
}

}  // namespace

namespace omm::ObjectTransformationWrapper
{

void define_python_interface(py::object& module)
{
  py::class_<ObjectTransformation>(module, wrapped_type::TYPE)
      .def("rotation", &ObjectTransformation::rotation)
      .def("scaling", [](const ObjectTransformation& ot) {
        return arma::conv_to<std::vector<double>>::from(ot.scaling());
      })
      .def("translation", [](const ObjectTransformation& ot) {
        return arma::conv_to<std::vector<double>>::from(ot.translation());
      })
      .def("set_rotation", &ObjectTransformation::set_rotation)
      .def("set_scaling", [](ObjectTransformation& ot, py::object value) {
        const auto [ret, vec] = to_vec2(value);
        if (ret) {
          ot.set_scaling(vec);
        }
        return ret;
      })
      .def("set_translation", [](ObjectTransformation& ot, py::object value) {
        const auto [ret, vec] = to_vec2(value);
        if (ret) {
          ot.set_translation(vec);
        }
        return ret;
      });
}

}  // namespace omm
