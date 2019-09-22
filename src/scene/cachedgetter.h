#pragma once
#include <map>
#include <tuple>

template<typename T, typename Self>
class CachedGetter
{

public:
  explicit CachedGetter(const Self& self) : m_self(self) {}
  const T& operator()() const
  {
    if (m_is_dirty) {
      m_cache = compute();
      m_is_dirty = false;
    }
    return m_cache;
  }

  void invalidate() const { m_is_dirty = true; }

protected:
  virtual T compute() const = 0;
  const Self& m_self;

private:
  mutable T m_cache;
  mutable bool m_is_dirty = false;

};
