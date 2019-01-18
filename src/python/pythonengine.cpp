#include <pybind11/embed.h>
#include <iostream>
#include "python/pythonengine.h"

#include "scene/scene.h"
#include "tags/scripttag.h"
#include "python/tagwrapper.h"
#include "python/scenewrapper.h"

namespace py = pybind11;

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

  py::object omm_module = py::module::import("omm");
  register_wrappers(omm_module);
}

bool PythonEngine::run(const std::string& code, const py::object& locals) const
{
  try {
    py::exec(code, py::globals(), locals);
    return true;
  } catch (const std::exception& e) {
    LOG(WARNING) << e.what();
    return false;
  }
}

// TODO imported symbols are not available inside `lambda`s or `def`s.

}  // namespace omm
