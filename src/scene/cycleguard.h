#pragma once

#include <memory>
#include <set>

namespace omm
{
class Object;
class CycleGuard
{
public:
  explicit CycleGuard(std::set<const Object*>& guarded_objects, const Object* guarded);
  ~CycleGuard();
  [[nodiscard]] bool inside_cycle() const
  {
    return m_inside_cycle;
  }

private:
  std::set<const Object*>& m_guarded_objects;
  const Object* const m_guarded;
  const bool m_inside_cycle;
};

}  // namespace omm
