#include "python/pathwrapper.h"
#include "python/pathpointwrapper.h"
#include "objects/path.h"

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
  auto& path = dynamic_cast<wrapped_type&>(wrapped);
  std::vector<PathPointWrapper> point_wrappers;
  point_wrappers.reserve(path.point_count());
  for (PathPoint* point : path.points()) {
    point_wrappers.emplace_back(*point);
  }
  return py::cast(point_wrappers);
}

}  // namespace omm
