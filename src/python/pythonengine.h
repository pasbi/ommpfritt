#pragma once

#include <string>
#include <pybind11/embed.h>
#include "observed.h"
#include "python/scopedinterpreterwrapper.h"

namespace omm
{

class Scene;
class AbstractPropertyOwner;

class PythonEngine : public QObject
{
  Q_OBJECT
public:
  explicit PythonEngine();
  bool
  exec(const std::string& code, const pybind11::object& locals, const void* association);
  pybind11::object
  eval(const std::string& code, const pybind11::object& locals, const void* association);

private:
  // the scoped_interpeter has same lifetime as the application.
  // otherwise, e.g., importing numpy causes crashed.
  // see https://pybind11.readthedocs.io/en/stable/advanced/embedding.html#interpreter-lifetime
  ScopedInterpreterWrapper m_guard;

  PythonEngine(const PythonEngine&) = delete;
  PythonEngine(PythonEngine&&) = delete;

Q_SIGNALS:
  void output(const void* associated_item, const std::string& text, Stream stream);
};

void register_wrappers(pybind11::object& module);

}  // namespace omm
