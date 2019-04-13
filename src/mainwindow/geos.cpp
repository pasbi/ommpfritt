#include "geos.h"
#include "logging.h"

namespace
{

void notice_function(const char* message, ...) { LINFO << "notice: " << message; }
void error_function(const char* message, ...) { LERROR << "error: " << message; }

}

namespace omm
{

Geos* Geos::m_instance = nullptr;

Geos::Geos()
{
  if (m_instance == nullptr) {
    m_instance = this;
  } else {
    LFATAL("Resetting application instance.");
  }
  m_handle = initGEOS_r(notice_function, error_function);
}

Geos::~Geos()
{
  m_instance = nullptr;
  finishGEOS_r(m_handle);
}

Geos &Geos::instance()
{
  assert(m_instance != nullptr);
  return *m_instance;
}

GEOSContextHandle_t Geos::handle() const { return m_handle; }

}  // namespace
