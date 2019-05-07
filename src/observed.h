#pragma once

#include <set>
#include <type_traits>
#include "common.h"
#include "logging.h"

template<typename ObserverT> class Observed
{
public:
  Observed(const Observed&) { }  // intentionally don't copy observers
  Observed() = default;
  virtual ~Observed() = default;

  void register_observer(ObserverT& observer)
  {
//    LINFO << m_observers.size() << " " << m_observers.count(&observer);
    assert(m_observers.count(&observer) == 0);
    m_observers.insert(&observer);
    assert(m_observers.count(&observer) == 1);
  }

  void unregister_observer(ObserverT& observer)
  {
//    LINFO << m_observers.size() << " " << m_observers.count(&observer);
    assert(m_observers.count(&observer) == 1);
    m_observers.erase(m_observers.find(&observer));
    assert(m_observers.count(&observer) == 0);
  }

  template<typename F> void for_each(F&& f)
  {
    std::for_each(m_observers.begin(), m_observers.end(), std::forward<F>(f));
  }

  template<typename F> void for_each(F&& f) const
  {
    std::for_each(m_observers.cbegin(), m_observers.cend(), std::forward<F>(f));
  }

  template<typename Result, typename F> auto transform(F&& f)
  {
    return ::transform<Result>(m_observers, std::forward<F>(f));
  }

  bool is_registered(ObserverT& observer) const
  {
    return ::contains(m_observers, &observer);
  }

private:
  std::set<ObserverT*> m_observers;
};
