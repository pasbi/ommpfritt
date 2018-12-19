#include "python/scenepywrapper.h"

#include <pybind11/stl.h>
#include "scene/scene.h"

namespace omm
{

ScenePyWrapper::ScenePyWrapper(Scene& scene) : m_wrapped(&scene) {}

template<typename T> py::object ScenePyWrapper::find_items(const std::string& name)
{
  return py::cast(::transform<PyWrapper<T>>(m_wrapped->find_items<T>(name), [](T* t) {
    return PyWrapper<T>(*t);
  }));
}

template py::object ScenePyWrapper::find_items<Object>(const std::string&);
template py::object ScenePyWrapper::find_items<Style>(const std::string&);
template py::object ScenePyWrapper::find_items<Tag>(const std::string&);

}  // namespace omm
