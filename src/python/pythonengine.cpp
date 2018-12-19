#include <pybind11/embed.h>
#include <iostream>
#include "python/pythonengine.h"

#include "scene/scene.h"
#include "tags/scripttag.h"
#include "python/itempywrapper.h"
#include "python/scenepywrapper.h"

namespace py = pybind11;

namespace
{

template<typename T> bool run(const std::string& code, T&& locals)
{
  try {
    py::exec(code, py::globals(), locals);
    return true;
  } catch (const std::exception& e) {
    LOG(WARNING) << e.what();
    return false;
  }
}

}  // namespace

namespace omm
{

PYBIND11_EMBEDDED_MODULE(omm, m) {
  py::class_<PropertyOwnerPyWrapper>(m, "property_owner")
      .def("get_property", &PropertyOwnerPyWrapper::get_property)
      .def("set_property", &PropertyOwnerPyWrapper::set_property);
  py::class_<PyWrapper<Object>, PropertyOwnerPyWrapper>(m, "object")
      .def("get_children", &PyWrapper<Object>::get_children)
      .def("get_parent", &PyWrapper<Object>::get_parent)
      .def("get_tags", &PyWrapper<Object>::get_tags);
  py::class_<PyWrapper<Tag>, PropertyOwnerPyWrapper>(m, "tag")
      .def("get_owner", &PyWrapper<Tag>::get_owner);
  py::class_<PyWrapper<Style>, PropertyOwnerPyWrapper>(m, "style");
  py::class_<ScenePyWrapper>(m, "scene")
      .def("find_tags", &ScenePyWrapper::find_items<Tag>)
      .def("find_objects", &ScenePyWrapper::find_items<Object>)
      .def("find_styles", &ScenePyWrapper::find_items<Style>);
}

PythonEngine::PythonEngine()
  : m_guard {}
{
  static size_t count = 0;
  if (count > 0) {
    LOG(FATAL) << "There must be not more than one PythonEngine.";
  }
  count++;

  py::module::import("omm");
}

PythonEngine::~PythonEngine()
{

}

void PythonEngine::run(Scene& scene) const
{
  using namespace py::literals;
  for (Tag* tag : scene.tags()) {
    if (tag->type() == ScriptTag::TYPE) {
      const auto code = tag->property(ScriptTag::CODE_PROPERTY_KEY).value<std::string>();
      auto locals = py::dict("this"_a=PyWrapper<Tag>(*tag), "scene"_a=ScenePyWrapper(scene));
      ::run(code, locals);
    }
  }
}

}  // namespace omm