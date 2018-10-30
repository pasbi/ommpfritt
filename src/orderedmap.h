#pragma once

#include <map>
#include <vector>

template<typename KeyT, typename ValueT>
class OrderedMap
{
public:
  ValueT& at(const KeyT& key) { return m_values.at(key); }
  const ValueT& at(const KeyT& key) const { return m_values.at(key); }
  const std::vector<KeyT>& keys() const { return m_keys; }
  bool contains(const KeyT& key) const { return m_values.count(key) == 1; }

  bool insert(const KeyT& key, ValueT&& value)
  {
    const auto info = m_values.insert(std::make_pair(key, std::forward<ValueT>(value)));
    const bool was_inserted = info.second;
    if (was_inserted) {
      m_keys.push_back(key);
      return true;
    } else {
      return false;
    }
  }

  auto begin() { return m_keys.begin(); }
  auto end() { return m_keys.end(); }
  auto begin() const { return m_keys.begin(); }
  auto end() const { return m_keys.end(); }
  size_t size() const { return m_keys.size(); }


private:
  std::vector<KeyT> m_keys;
  std::map<KeyT, ValueT> m_values;
};
