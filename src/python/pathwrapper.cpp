#include "python/pathwrapper.h"
#include "python/pathpointwrapper.h"
#include "objects/pathobject.h"
#include "path/pathvector.h"

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
  auto& path_object = dynamic_cast<wrapped_type&>(wrapped);
  std::vector<PathPointWrapper> point_wrappers;
  point_wrappers.reserve(path_object.geometry().point_count());
  for (PathPoint* point : path_object.geometry().points()) {
    point_wrappers.emplace_back(*point);
  }
  return py::cast(point_wrappers);
}

}  // namespace omm
