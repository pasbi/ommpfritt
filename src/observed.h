#pragma once

#include <set>
#include <type_traits>
#include "common.h"

template<typename ObserverT> class Observed
{
public:
  void register_observer(ObserverT& observer)
  {
    assert(m_observers.count(&observer) == 0);
    m_observers.insert(&observer);
  }

  void unregister_observer(ObserverT& observer)
  {
    assert(m_observers.count(&observer) == 1);
    m_observers.erase(m_observers.find(&observer));
  }

  template<typename F> void for_each(F&& f)
  {
    std::for_each(m_observers.begin(), m_observers.end(), std::forward<F>(f));
  }

  template<typename Result, typename F> auto transform(F&& f)
  {
    return ::transform<Result>(m_observers, std::forward<F>(f));
  }

private:
  std::set<ObserverT*> m_observers;
};
