#pragma once

#include <map>
#include <vector>

template<typename KeyT, typename ValueT>
class OrderedMap
{
public:
  using key_type = KeyT;
  using mapped_type = ValueT;
  using value_type = std::pair<key_type, mapped_type>;
  mapped_type& at(const key_type& key) { return m_values.at(key); }
  const mapped_type& at(const key_type& key) const { return m_values.at(key); }
  const std::vector<key_type>& keys() const { return m_keys; }
  bool contains(const key_type& key) const { return m_values.count(key) == 1; }

  template<typename mapped_type_>
  bool insert(const key_type& key, mapped_type_&& value)
  {
    constexpr bool template_argument_is_valid = std::is_same< std::decay_t<mapped_type_>,
                                                              std::decay_t<mapped_type > >::value;
    static_assert(template_argument_is_valid, "mapped_type_ and mapped_type must be same.");

    bool was_inserted;
    std::tie(std::ignore, was_inserted)
      = m_values.insert(std::make_pair(key, std::forward<mapped_type_>(value)));
    if (was_inserted) {
      m_keys.push_back(key);
      return true;
    } else {
      return false;
    }
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

  size_t size() const { return m_keys.size(); }


private:
  std::vector<key_type> m_keys;
  std::map<key_type, mapped_type> m_values;
};
