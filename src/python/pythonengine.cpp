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
}


PythonEngine::PythonEngine()
  : m_guard {}
{
  static size_t count = 0;
  if (count > 0) {
    LOG(FATAL) << "There must be not more than one PythonEngine.";
  }
  count++;

  py::object m = py::module::import("omm");
  py::class_<PropertyOwnerPyWrapper>(m, "property_owner")
      .def("property", &PropertyOwnerPyWrapper::property)
      .def("set", &PropertyOwnerPyWrapper::set);
  py::class_<PyWrapper<Object>, PropertyOwnerPyWrapper>(m, "object")
      .def("children", &PyWrapper<Object>::children)
      .def("parent", &PyWrapper<Object>::parent)
      .def("tags", &PyWrapper<Object>::tags)
      .def("type", &PyWrapper<Object>::type);
  py::class_<PyWrapper<Tag>, PropertyOwnerPyWrapper>(m, "tag")
      .def("owner", &PyWrapper<Tag>::owner)
      .def("type", &PyWrapper<Tag>::type);
  py::class_<PyWrapper<Style>, PropertyOwnerPyWrapper>(m, "style");
  py::class_<ScenePyWrapper>(m, "scene")
      .def("find_tags", &ScenePyWrapper::find_items<Tag>)
      .def("find_objects", &ScenePyWrapper::find_items<Object>)
      .def("find_styles", &ScenePyWrapper::find_items<Style>);
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