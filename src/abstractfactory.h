#pragma once

#include <map>
#include <memory>
#include <algorithm>
#include <set>
#include "aspects/typed.h"

template<typename Key, typename GeneralT, typename... Args>
class AbstractFactory : public virtual omm::Typed
{
protected:
  using creator_type = std::unique_ptr<GeneralT>(*)(Args&&... args);
  using creator_map_type = std::map<Key, creator_type>;
  virtual ~AbstractFactory() = default;


private:
  template<typename SpecialT>
  static std::unique_ptr<GeneralT> make(Args&&... args)
  {
    return std::make_unique<SpecialT>(std::forward<Args>(args)...);
  }

public:
  template<typename SpecialT> static void register_type(Key key)
  {
    auto pair = std::pair<Key, creator_type>(key, &make<SpecialT>);
    m_creator_map.insert(pair);
  }

  template<typename... Args_>
  static std::unique_ptr<GeneralT> make(const Key& key, Args_&&... args)
  {
    return m_creator_map.at(key)(std::forward<Args>(args)...);
  }

  static std::unique_ptr<GeneralT> make(const Key& key)
  {
    static_assert(sizeof...(Args) == 0, "Expected zero arguments.");
    return m_creator_map.at(key)();
  }

  static std::set<Key> keys()
  {
    std::set<Key> keys;
    std::transform( m_creator_map.begin(), m_creator_map.end(),
                    std::inserter(keys, keys.end()), [](const auto& pair) { return pair.first; } );
    return keys;
  }

private:
  static creator_map_type m_creator_map;
};

template<typename Key, typename GeneralT, typename... Args>
typename AbstractFactory<Key, GeneralT, Args...>::creator_map_type
AbstractFactory<Key, GeneralT, Args...>::m_creator_map;
