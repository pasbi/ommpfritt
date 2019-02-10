#include "python/pythonstreamredirect.h"

namespace omm
{

namespace py = pybind11;

PythonStreamRedirect::PythonStreamRedirect()
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

PythonStreamRedirect::~PythonStreamRedirect()
{
  auto sysm = py::module::import("sys");
  sysm.attr("stdout") = m_stdout;
  sysm.attr("stderr") = m_stderr;
}

std::string PythonStreamRedirect::stdout()
{
  m_stdout_buffer.attr("seek")(0);
  return py::str(m_stdout_buffer.attr("read")());
}

std::string PythonStreamRedirect::stderr()
{
  m_stderr_buffer.attr("seek")(0);
  return py::str(m_stderr_buffer.attr("read")());
}

}  // namespace omm
