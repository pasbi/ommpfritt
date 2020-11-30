#include "python/scopedinterpreterwrapper.h"
#include "external/pybind11/embed.h"

namespace omm
{
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
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
