#pragma once

#include <string>
#include <pybind11/embed.h>

namespace omm
{

class Scene;

class PythonEngine
{
public:
  explicit PythonEngine();
  ~PythonEngine();

private:
  pybind11::scoped_interpreter m_guard;

  PythonEngine(const PythonEngine&) = delete;
  PythonEngine(PythonEngine&&) = delete;
};

}  // namespace omm
