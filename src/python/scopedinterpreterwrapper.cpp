#include "python/scopedinterpreterwrapper.h"
#include <pybind11/embed.h>

namespace omm
{
ScopedInterpreterWrapper* ScopedInterpreterWrapper::m_instance = nullptr;

ScopedInterpreterWrapper::ScopedInterpreterWrapper()
    : m_scoped_interpreter(new pybind11::scoped_interpreter())
{
}

ScopedInterpreterWrapper::~ScopedInterpreterWrapper()
{
  delete static_cast<pybind11::scoped_interpreter*>(m_scoped_interpreter);
  m_scoped_interpreter = nullptr;
}

}  // namespace omm
