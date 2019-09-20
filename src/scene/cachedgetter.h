#pragma once
#include <map>
#include <tuple>

template<typename T, typename Self, typename... Args>
class CachedGetter
{

public:
  explicit CachedGetter(const Self& self) : m_self(self) {}
  T operator()(Args... args) const
  {
    auto args_tuple = std::tuple(args...);
    if (m_cache.find(args_tuple) == m_cache.end()) {
      m_cache[args_tuple] = compute(args...);
    }
    return m_cache[args_tuple];
  }

  void invalidate() const { m_cache.clear(); }

protected:
  virtual T compute(Args... args) const = 0;
  const Self& m_self;

private:
  mutable std::map<std::tuple<Args...>, T> m_cache;

};

template<typename T, typename Self>
class CachedGetter<T, Self, void>
{

public:
  explicit CachedGetter(const Self& self) : m_self(self) {}
  T operator()() const
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
