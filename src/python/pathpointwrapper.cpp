#include "python/pathpointwrapper.h"
#include "python/pointwrapper.h"
#include "external/pybind11/stl.h"

namespace omm
{
void PathPointWrapper::define_python_interface(py::object& module)
{
  py::class_<PathPointWrapper>(module, wrapped_type::TYPE)
      .def_property("geometry", &PathPointWrapper::geometry, &PathPointWrapper::set_geometry)
      ;
}

pybind11::object PathPointWrapper::geometry() const
{
  return py::cast(PointWrapper(wrapped.geometry()));
}

void PathPointWrapper::set_geometry(const pybind11::object& value)
{
  wrapped.set_geometry(value.cast<Point>());
}

}  // namespace omm
