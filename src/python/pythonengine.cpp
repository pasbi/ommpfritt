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

void notify(const std::string& text, const std::function<void(const std::string&)>& f)
{
  if (text.size() > 0) { f(text); }
}

auto on_stderr(omm::PythonIOObserver* observer, const void* associated_item)
{
  using namespace std::placeholders;
  return std::bind(&omm::PythonIOObserver::on_stderr, observer, associated_item, _1);
}

auto on_stdout(omm::PythonIOObserver* observer, const void* associated_item)
{
  using namespace std::placeholders;
  return std::bind(&omm::PythonIOObserver::on_stdout, observer, associated_item, _1);
}

}  // namespace

namespace omm
{

PYBIND11_EMBEDDED_MODULE(omm, m) { Q_UNUSED(m); }

PythonEngine::PythonEngine()
{
  static size_t count = 0;
  if (count > 0) {
    LFATAL("There must be not pymore than one PythonEngine.");
  }
  count++;

  py::object omm_module = py::module::import("omm");
  register_wrappers(omm_module);
}

bool PythonEngine
::exec(const std::string& code, const py::object& locals, const void* associated_item) const
{
  // actually, they are used. However some compilers emit false warnings without the following.
  Q_UNUSED(::on_stderr)
  Q_UNUSED(::on_stdout)
  Q_UNUSED(::notify)

  PythonStreamRedirect py_output_redirect {};
  try {
    py::exec(code, py::globals(), locals);
    Observed<PythonIOObserver>::for_each([&](auto* observer) {
      notify(py_output_redirect.stdout_(), on_stdout(observer, associated_item));
      notify(py_output_redirect.stderr_(), on_stderr(observer, associated_item));
    });
    return true;
  } catch (const std::exception& e) {
    Observed<PythonIOObserver>::for_each([&](auto* observer) {
      notify(e.what(), on_stderr(observer, associated_item));
    });
    return false;
  }
}

pybind11::object PythonEngine
::eval(const std::string& code, const py::object& locals, const void* associated_item) const
{

  PythonStreamRedirect py_output_redirect {};
  try {
    auto result = py::eval(code, py::globals(), locals);
    Observed<PythonIOObserver>::for_each([&](auto* observer) {
      notify(py_output_redirect.stdout_(), on_stdout(observer, associated_item));
      notify(py_output_redirect.stderr_(), on_stderr(observer, associated_item));
    });
    return result;
  } catch (const std::exception& e) {
    Observed<PythonIOObserver>::for_each([&](auto* observer) {
      notify(e.what(), on_stderr(observer, associated_item));
    });
    return py::none();
  }
}

// TODO imported symbols are not available inside `lambda`s or `def`s.

}  // namespace omm

// TODO after copying script tag to another object: owner() yields wrong object
