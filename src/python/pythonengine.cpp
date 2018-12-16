#include <pybind11/embed.h>
#include <iostream>
#include "pythonengine.h"

#include "scene/scene.h"
#include "python/objectview.h"
#include "python/tagview.h"

namespace py = pybind11;

namespace omm
{

PythonEngine::PythonEngine()
  : m_guard {}
{
  static size_t count = 0;
  if (count > 0) {
    LOG(FATAL) << "There must be not more than one PythonEngine.";
  }
  count++;
}

PythonEngine::~PythonEngine()
{

}

}  // namespace omm