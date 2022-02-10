#pragma once

#include "common.h"
#include <map>
#include <type_traits>
#include <vector>

template<typename KeyT, typename ValueT> class OrderedMap
{
public:
  using key_type = KeyT;
  using mapped_type = std::unique_ptr<ValueT>;
  using value_type = std::pair<key_type, mapped_type>;
  mapped_type& at(const key_type& key)
  {
    return m_values.at(key);
  }
  const mapped_type& at(const key_type& key) const
  {
    return m_values.at(key);
  }
  const std::vector<key_type>& keys() const
  {
    return m_keys;
  }
  explicit OrderedMap(const OrderedMap<KeyT, ValueT>& other)
  {
    for (auto&& key : other.m_keys) {
      insert(key, other.at(key)->clone());
    }
  }

  explicit OrderedMap() = default;
  explicit OrderedMap(OrderedMap<KeyT, ValueT>&& other) noexcept = default;
  ~OrderedMap() = default;
  OrderedMap& operator=(const OrderedMap&) = delete;
  OrderedMap& operator=(OrderedMap&&) = delete;

  auto values() const
  {
    if constexpr (::is_unique_ptr<mapped_type>::value) {
      return util::transform(keys(), [this](const KeyT& key) { return m_values.at(key).get(); });
    } else {
      return util::transform(keys(), [this](const KeyT& key) { return &m_values.at(key); });
    }
  }

  bool contains(const key_type& key) const
  {
    return m_values.count(key) == 1;
  }

  template<typename mapped_type_> bool insert(const key_type& key, mapped_type_&& value)
  {
    const auto [_, was_inserted]
        = m_values.insert(std::pair(key, std::forward<mapped_type_>(value)));
    if (was_inserted) {
      m_keys.push_back(key);
      return true;
    } else {
      return false;
    }
  }

  mapped_type extract(const key_type key)
  {
    m_keys.erase(remove(m_keys.begin(), m_keys.end(), key), m_keys.end());
    auto v = std::move(m_values.extract(key).mapped());
    assert(m_values.count(key) == 0);
    assert(std::find(m_keys.begin(), m_keys.end(), key) == m_keys.end());
    return v;
  }

  void clear()
  {
    m_keys.clear();
    m_values.clear();
  }

#if 0
// TODO replace the OrderedMap::insert with something like that:
// reason: supply a insert(value_type&&) overload
  template<typename mapped_type_>
  bool insert(const key_type& key, mapped_type&& value)
  {
    auto pair = std::make_pair(key, value);
    return insert<std::pair<key_type, mapped_type_>>(std::forward<mapped_type>(pair));
  }

  template<typename value_type_>
  bool insert(value_type_&& value)
  {
    static_assert( std::is_same<std::decay_t<value_type_>, std::decay_t<value_type>>::value,
                   "value_type_ and value_type must be same.");
    bool was_inserted;
    std::tie(std::ignore, was_inserted) = m_values.insert(std::forward<value_type_>(value));
    if (was_inserted) {
      m_keys.push_back(value);
      return true;
    } else {
      return false;
    }
  }
#endif

  [[nodiscard]] std::size_t size() const
  {
    return m_keys.size();
  }

private:
  std::vector<key_type> m_keys;
  std::map<key_type, mapped_type> m_values;
};
