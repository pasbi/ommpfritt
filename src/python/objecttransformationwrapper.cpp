#include "python/objecttransformationwrapper.h"
#include <pybind11/stl.h>
#include <armadillo>
#include "common.h"

namespace py = pybind11;

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
        ot.set_scaling(to_vec2(value));
      })
      .def("set_translation", [](ObjectTransformation& ot, py::object value) {
        ot.set_translation(to_vec2(value));
      });
}

}  // namespace omm
