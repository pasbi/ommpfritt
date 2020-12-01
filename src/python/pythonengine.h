#pragma once

#include "common.h"
#include "external/pybind11/embed.h"
#include "python/scopedinterpreterwrapper.h"
#include <QObject>
#include <string>

namespace omm
{
class Scene;
class AbstractPropertyOwner;

class PythonEngine : public QObject
{
  Q_OBJECT
public:
  explicit PythonEngine();
  PythonEngine(PythonEngine&&) = delete;
  PythonEngine(const PythonEngine&) = delete;
  PythonEngine& operator=(PythonEngine&&) = delete;
  PythonEngine& operator=(const PythonEngine&) = delete;
  ~PythonEngine() override = default;
  bool exec(const QString& code, pybind11::object& locals, const void* associated_item);
  pybind11::object eval(const QString& code, pybind11::object& locals, const void* associated_item);

private:
  // the scoped_interpeter has same lifetime as the application.
  // otherwise, e.g., importing numpy causes crashed.
  // see https://pybind11.readthedocs.io/en/stable/advanced/embedding.html#interpreter-lifetime
  ScopedInterpreterWrapper m_guard;

Q_SIGNALS:
  void output(const void* associated_item, const QString& text, Stream stream);
};

void register_wrappers(pybind11::object& module);

}  // namespace omm
