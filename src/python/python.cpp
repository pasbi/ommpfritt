#include <pybind11/embed.h>
#include <iostream>
#include "python.h"

#include "scene/scene.h"
#include "python/objectview.h"
#include "python/tagview.h"

namespace py = pybind11;
using namespace pybind11::literals;

omm::Python::Python(ConstructorTag)
{
}

// PYBIND11_EMBEDDED_MODULE(ommpfritt, m) {
//     py::class_<omm::Scene>(m, "Scene")
//       .def("current", &omm::Scene::currentInstance, py::return_value_policy::reference)
//       .def("root", &omm::Scene::root_view)
//       ;

//     py::class_<omm::ObjectView>(m, "Object")
//       .def("property", &omm::ObjectView::get_property)
//       .def("set_property", &omm::ObjectView::set_property)
//       .def("children", &omm::ObjectView::children)
//       .def("parent", &omm::ObjectView::parent)
//       .def("tags", &omm::ObjectView::tags)
//       .def("remove", &omm::ObjectView::remove)
//       ;

//     py::class_<omm::TagView>(m, "Tag")
//       .def("property", &omm::TagView::get_property)
//       .def("set_property", &omm::TagView::set_property)
//       .def("owner", &omm::TagView::owner)
//       .def("remove", &omm::TagView::remove)
//       ;
// }

bool omm::Python::run(omm::Scene& scene, const std::string code)
{
  // py::scoped_interpreter guard {};

  // auto locals = py::dict();

  // try {
  //   py::exec(code, py::globals(), locals);
  //   return true;
  // } catch (const pybind11::error_already_set& e) {
  //   LOG(WARNING) << e.what();
  // }
  return false;
}

omm::Python& omm::Python::instance()
{
  static std::unique_ptr<Python> instance(nullptr);
  if (instance == nullptr) {
    instance = std::make_unique<Python>(Python::ConstructorTag{});
  }
  return *instance;
}
