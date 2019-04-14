#include "python/pathwrapper.h"
#include "python/pointwrapper.h"

namespace omm
{

void PathWrapper::define_python_interface(py::object& module)
{
  ObjectWrapper::register_wrapper<PathWrapper>();
  py::class_<PathWrapper, ObjectWrapper>(module, wrapped_type::TYPE)
      .def("points", &PathWrapper::points);
}

py::object PathWrapper::points()
{
  auto points = static_cast<wrapped_type&>(wrapped).points();
  std::vector<PointWrapper> point_wrappers;
  point_wrappers.reserve(points.size());
  for (Point& point : points) {
    point_wrappers.emplace_back(point);
  }
  return py::cast(point_wrappers);
}

}  // namespace omm
