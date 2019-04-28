#pragma once

#include <pybind11/pybind11.h>

// https://github.com/pybind/pybind11/issues/1622#issuecomment-452718093

namespace omm
{

class PythonStreamRedirect
{
public:
  PythonStreamRedirect();
  ~PythonStreamRedirect();
  std::string stdout_();
  std::string stderr_();

private:
  pybind11::object m_stdout;
  pybind11::object m_stderr;
  pybind11::object m_stdout_buffer;
  pybind11::object m_stderr_buffer;
};

}  // namespace omm
