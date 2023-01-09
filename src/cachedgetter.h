#pragma once

#include <map>
#include <tuple>
#include <functional>
#include <memory>
#include <utility>

template<typename T, typename Self, typename... Args> class ArgsCachedGetter
{
public:
  explicit ArgsCachedGetter(const Self& self) : m_self(self)
  {
  }
  const T& operator()(Args... args) const
  {
    const auto key = std::tuple(args...);
    if (m_cache.find(key) == m_cache.end()) {
      m_cache.insert(std::pair(key, compute(args...)));
    }
    return m_cache.at(key);
  }

  void invalidate()
  {
    m_cache.clear();
  }

protected:
  virtual T compute(Args... args) const = 0;
  const Self& m_self;

private:
  mutable std::map<std::tuple<Args...>, T> m_cache;
};

template<typename T, typename Self> class CachedGetter
{
public:
  explicit CachedGetter(const Self& self) : m_self(self)
  {
  }

  CachedGetter(const CachedGetter&) = delete;
  CachedGetter(CachedGetter&&) = delete;
  CachedGetter& operator=(const CachedGetter&) = delete;
  CachedGetter& operator=(CachedGetter&&) = delete;

  virtual ~CachedGetter() = default;

  const T& operator()() const
  {
    if (m_is_dirty) {
      m_cache = compute();
      m_is_dirty = false;
    }
    return m_cache;
  }

  void invalidate()
  {
    m_is_dirty = true;
  }

protected:
  virtual T compute() const = 0;
  const Self& m_self;

private:
  mutable T m_cache;
  mutable bool m_is_dirty = true;
};

template<typename Self, typename Compute>
auto make_simple_cached_getter(const Self& self, const Compute& compute)
{
  using R = decltype(std::invoke(std::declval<Compute>(), std::declval<Self>()));
  class SimpleCachedGetter
      : public CachedGetter<R, Self>
  {
  public:
    explicit SimpleCachedGetter(const Self& self, const Compute& compute)
        : CachedGetter<R, Self>(self)
        , m_compute(compute)
    {
    }

    R compute() const override
    {
      return std::invoke(m_compute, this->m_self);
    }

  private:
    Compute m_compute;
  };
  return static_cast<std::unique_ptr<CachedGetter<R, Self>>>(std::make_unique<SimpleCachedGetter>(self, compute));
}
