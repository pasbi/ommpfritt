#pragma once

#include <map>

namespace omm
{
/**
 * This class is deprecated. You should not use it.
 * Use CachedGetter instead. The class is currently being used but will be replaced with
 * CachedGetter soon.
 */
template<typename K, typename V> class Cache
{
public:
  Cache() = default;
  Cache(Cache&&) = delete;
  Cache(const Cache&) = delete;
  Cache& operator=(Cache&& other) = delete;
  Cache& operator=(const Cache& other) = delete;
  virtual ~Cache() = default;
  const V& get(const K& key) const
  {
    if (m_cache.count(key) == 0) {
      m_cache[key] = retrieve(key);
    }
    return m_cache.at(key);
  }
  void clear()
  {
    m_cache.clear();
  }

protected:
  virtual V retrieve(const K& key) const = 0;

private:
  mutable std::map<K, V> m_cache;
};

}  // namespace omm
