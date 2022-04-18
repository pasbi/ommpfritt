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
  static constexpr auto wrap = [](auto* path_point) { return PathPointWrapper{*path_point}; };
  const auto point_wrappers = util::transform<std::vector>(path_object.path_vector().points(), wrap);
  return py::cast(point_wrappers);
}

}  // namespace omm
