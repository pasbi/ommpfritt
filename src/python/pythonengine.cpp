#include <pybind11/embed.h>
#include <iostream>
#include "python/pythonengine.h"

#include "scene/scene.h"
#include "tags/scripttag.h"
#include "python/pywrapper.h"

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

  py::class_<PropertyOwnerWrapper>(m, "PropertyOwner")
        .def("property", &PropertyOwnerWrapper::property)
        .def("set", &PropertyOwnerWrapper::set);

  py::class_<TagWrapper, PropertyOwnerWrapper>(m, "Tag")
        .def("owner", &TagWrapper::owner);

  py::class_<ScriptTagWrapper, TagWrapper>(m, "ScriptTag");
  
  py::class_<ObjectWrapper, PropertyOwnerWrapper>(m, "Object")
      .def("children", &ObjectWrapper::children)
      .def("parent", &ObjectWrapper::parent)
      .def("tags", &ObjectWrapper::tags);

  py::class_<StyleWrapper, PropertyOwnerWrapper>(m, "style");

  py::class_<SceneWrapper>(m, "scene")
      .def("find_tags", &SceneWrapper::find_items<Tag>)
      .def("find_objects", &SceneWrapper::find_items<Object>)
      .def("find_styles", &SceneWrapper::find_items<Style>);
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
      auto locals = py::dict("this"_a=TagWrapper(tag)); //, "scene"_a=ScenePyWrapper(scene));
      ::run(code, locals);
    }
  }
}

}  // namespace omm