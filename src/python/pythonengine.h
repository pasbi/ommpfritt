#pragma once

#pragma once

#include <string>
#include <pybind11/embed.h>
#include "observed.h"

namespace omm
{

class Scene;
class AbstractPropertyOwner;

class PythonIOObserver
{
public:
  virtual ~PythonIOObserver() = default;
  virtual void on_stdout(const void* item, const std::string& text) = 0;
  virtual void on_stderr(const void* item, const std::string& text) = 0;
};

class PythonEngine : public Observed<PythonIOObserver>
{
public:
  explicit PythonEngine();
  bool
  exec(const std::string& code, const pybind11::object& locals, const void* association) const;
  pybind11::object
  eval(const std::string& code, const pybind11::object& locals, const void* association) const;

private:

  // the scoped_interpeter has same lifetime as the application.
  // otherwise, e.g., importing numpy causes crashed.
  // see https://pybind11.readthedocs.io/en/stable/advanced/embedding.html#interpreter-lifetime
  pybind11::scoped_interpreter m_guard {};

  PythonEngine(const PythonEngine&) = delete;
  PythonEngine(PythonEngine&&) = delete;
};

void register_wrappers(pybind11::object& module);

}  // namespace omm
