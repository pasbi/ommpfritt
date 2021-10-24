#include "python/scenewrapper.h"
#include "python/objectwrapper.h"
#include "python/stylewrapper.h"
#include "python/tagwrapper.h"
#include "scene/scene.h"

namespace omm
{
void SceneWrapper::define_python_interface(py::object& module)
{
  py::class_<SceneWrapper>(module, wrapped_type::TYPE)
      .def("find_tags", &SceneWrapper::find_items<Tag>)
      .def("find_objects", &SceneWrapper::find_items<Object>)
      .def("find_styles", &SceneWrapper::find_items<Style>);
}

template<typename T> py::object SceneWrapper::find_items(const QString& name) const
{
  return wrap(wrapped.find_items<T>(name));
}

template py::object SceneWrapper::find_items<Object>(const QString&) const;
template py::object SceneWrapper::find_items<Tag>(const QString&) const;
template py::object SceneWrapper::find_items<Style>(const QString&) const;

}  // namespace omm
