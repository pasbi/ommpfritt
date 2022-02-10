#include "scene/cycleguard.h"
#include "common.h"
#include "logging.h"

namespace omm
{
CycleGuard::CycleGuard(std::set<const Object*>& guarded_objects, const Object* guarded)
    : m_guarded_objects(guarded_objects), m_guarded(guarded),
      m_inside_cycle(m_guarded_objects.contains(m_guarded))
{
  m_guarded_objects.insert(m_guarded);
}

CycleGuard::~CycleGuard()
{
  if (m_inside_cycle) {
    // if we were inside a cycle, then m_guarded was already in m_guarded_objects.
    // we must not remove it to be able to detect other cycles properly.
  } else {
    m_guarded_objects.erase(m_guarded);
  }
}

}  // namespace omm
