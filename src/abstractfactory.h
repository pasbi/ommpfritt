#pragma once

#include <map>
#include <memory>
#include <algorithm>
#include <set>
#include "aspects/typed.h"

template<typename Key, bool enable_clone, typename GeneralT, typename... Args>
class AbstractFactory : public virtual omm::Typed
{
protected:
  using creator_type = std::unique_ptr<GeneralT>(*)(Args&&...);
  using creator_map_type = std::map<Key, creator_type>;
  using clone_type = std::unique_ptr<GeneralT>(*)(const GeneralT&);
  using clone_map_type = std::map<Key, clone_type>;
  virtual ~AbstractFactory() = default;

private:
  template<typename SpecialT>
  static std::unique_ptr<GeneralT> make_(Args&&... args)
  {
    return std::make_unique<SpecialT>(std::forward<Args>(args)...);
  }

  template<typename SpecialT>
  static std::unique_ptr<GeneralT> clone_(const GeneralT& original)
  {
    if constexpr (std::is_same_v<decltype(SpecialT::TYPE), QString()>) {
      assert(original.type() == SpecialT::TYPE());
    } else {
      assert(original.type() == SpecialT::TYPE);
    }

    return std::make_unique<SpecialT>(static_cast<const SpecialT&>(original));
  }

public:
  using factory_item_type = GeneralT;
  template<typename SpecialT> static void register_type(Key key)
  {
    m_creator_map[key] = &make_<SpecialT>;
    if constexpr (enable_clone) {
      m_clone_map[key] = &clone_<SpecialT>;
    }
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

  std::unique_ptr<GeneralT> clone() const
  {
    if constexpr (enable_clone) {
      return m_clone_map.at(type())(static_cast<const GeneralT&>(*this));
    } else {
      Q_UNREACHABLE();
      return nullptr;
    }
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
  static clone_map_type m_clone_map;

};

template<typename Key, bool enable_clone, typename GeneralT, typename... Args>
typename AbstractFactory<Key, enable_clone, GeneralT, Args...>::creator_map_type
AbstractFactory<Key, enable_clone, GeneralT, Args...>::m_creator_map;

template<typename Key, bool enable_clone, typename GeneralT, typename... Args>
typename AbstractFactory<Key, enable_clone, GeneralT, Args...>::clone_map_type
AbstractFactory<Key, enable_clone, GeneralT, Args...>::m_clone_map;
