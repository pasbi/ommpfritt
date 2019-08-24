#include <pybind11/embed.h>
#include <iostream>
#include <pybind11/iostream.h>
#include <functional>
#include "python/pythonengine.h"
#include "scene/scene.h"
#include "tags/scripttag.h"
#include "python/tagwrapper.h"
#include "python/scenewrapper.h"

namespace py = pybind11;

namespace
{

class PythonStreamRedirect
{
public:
  PythonStreamRedirect()
  {
    auto sysm = py::module::import("sys");
    m_stdout = sysm.attr("stdout");
    m_stderr = sysm.attr("stderr");
    auto stringio = py::module::import("io").attr("StringIO");
    m_stdout_buffer = stringio();
    m_stderr_buffer = stringio();
    sysm.attr("stdout") = m_stdout_buffer;
    sysm.attr("stderr") = m_stderr_buffer;
  }

  ~PythonStreamRedirect()
  {
    auto sysm = py::module::import("sys");
    sysm.attr("stdout") = m_stdout;
    sysm.attr("stderr") = m_stderr;
  }

  std::string stdout_()
  {
    m_stdout_buffer.attr("seek")(0);
    return py::str(m_stdout_buffer.attr("read")());
  }

  std::string stderr_()
  {
    m_stderr_buffer.attr("seek")(0);
    return py::str(m_stderr_buffer.attr("read")());
  }

private:
  pybind11::object m_stdout;
  pybind11::object m_stderr;
  pybind11::object m_stdout_buffer;
  pybind11::object m_stderr_buffer;
};

}  // namespace

namespace omm
{

PYBIND11_EMBEDDED_MODULE(omm, m) { Q_UNUSED(m); }

PythonEngine::PythonEngine()
{
  static size_t count = 0;
  if (count > 0) {
    LFATAL("There must be not more than one PythonEngine.");
  }
  count++;

  py::object omm_module = py::module::import("omm");
  register_wrappers(omm_module);
}

bool PythonEngine
::exec(const std::string& code, const py::object& locals, const void* associated_item)
{
  PythonStreamRedirect py_output_redirect {};
  try {
    py::exec(code, py::globals(), locals);
    Q_EMIT output(associated_item, py_output_redirect.stdout_(), Stream::stdout_);
    Q_EMIT output(associated_item, py_output_redirect.stderr_(), Stream::stderr_);
    return true;
  } catch (const std::exception& e) {
    Q_EMIT output(associated_item, e.what(), Stream::stderr_);
    return false;
  }
}

pybind11::object PythonEngine
::eval(const std::string& code, const py::object& locals, const void* associated_item)
{
  PythonStreamRedirect py_output_redirect {};
  try {
    auto result = py::eval(code, py::globals(), locals);
    Q_EMIT output(associated_item, py_output_redirect.stdout_(), Stream::stdout_);
    Q_EMIT output(associated_item, py_output_redirect.stderr_(), Stream::stderr_);
    return result;
  } catch (const std::exception& e) {
    Q_EMIT output(associated_item, e.what(), Stream::stderr_);
    return py::none();
  }
}

// TODO imported symbols are not available inside `lambda`s or `def`s.

}  // namespace omm

// TODO after copying script tag to another object: owner() yields wrong object
