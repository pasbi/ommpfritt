#pragma once

#include "common.h"
#include "external/pybind11/embed.h"
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
  PythonEngine(PythonEngine&&) = delete;
  PythonEngine(const PythonEngine&) = delete;
  PythonEngine& operator=(PythonEngine&&) = delete;
  PythonEngine& operator=(const PythonEngine&) = delete;
  bool exec(const QString& code, pybind11::object& locals, const void* associated_item);
  pybind11::object eval(const QString& code, pybind11::object& locals, const void* associated_item);
  static PythonEngine& instance();

Q_SIGNALS:
  void output(const void* associated_item, const QString& text, Stream stream);

private:
  explicit PythonEngine();
  ~PythonEngine() override;
  // use void* to avoid compiler warning 'declared with greater visibility than the type of its field`
  void* m_scoped_interpreter;
};

void register_wrappers(pybind11::object& module);

}  // namespace omm
