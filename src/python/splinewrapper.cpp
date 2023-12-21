#include "python/splinewrapper.h"
#include "geometry/vec2.h"
#include <pybind11/stl.h>

namespace omm
{
void SplineWrapper::define_python_interface(py::object& module)
{
  py::class_<SplineType>(module, "SplineType")
      .def("value", [](const SplineType& spline, double t) { return spline.evaluate(t).value(); })
      .def("derivative",
           [](const SplineType& spline, double t) { return spline.evaluate(t).derivative(); });
}

}  // namespace omm
