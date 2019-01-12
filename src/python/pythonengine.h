#pragma once

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

  void prepare_frame(Scene& scene) const;
  void evaluate_script_tags(Scene& scene, bool force = true) const;
  bool run(const std::string& code, const pybind11::object& locals) const;

private:
  // the scoped_interpeter has same lifetime as the application.
  // otherwise, e.g., importing numpy causes crashed.
  // see https://pybind11.readthedocs.io/en/stable/advanced/embedding.html#interpreter-lifetime
  pybind11::scoped_interpreter m_guard;

  PythonEngine(const PythonEngine&) = delete;
  PythonEngine(PythonEngine&&) = delete;
};

void register_wrappers(pybind11::object& module);

}  // namespace omm
