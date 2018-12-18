#include <pybind11/embed.h>
#include <iostream>
#include "python/pythonengine.h"

#include "scene/scene.h"
#include "tags/scripttag.h"
#include "python/tagpywrapper.h"
#include "python/objectpywrapper.h"

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
  py::class_<ObjectPyWrapper>(m, "object")
      .def("get_property", &ObjectPyWrapper::get_property)
      .def("set_property", &ObjectPyWrapper::set_property)
      .def("get_children", &ObjectPyWrapper::get_children)
      .def("get_parent", &ObjectPyWrapper::get_parent);
  py::class_<TagPyWrapper>(m, "tag")
      .def("get_owner", &TagPyWrapper::get_owner)
      .def("get_property", &TagPyWrapper::get_property)
      .def("set_property", &TagPyWrapper::set_property);
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
      auto locals = py::dict("this"_a=TagPyWrapper(*tag));
      ::run(code, locals);
    }
  }
}

}  // namespace omm