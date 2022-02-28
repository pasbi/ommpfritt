#pragma once

#include "aspects/typed.h"
#include <QObject>
#include <algorithm>
#include <map>
#include <memory>
#include <set>

template<typename Key, bool enable_clone, typename GeneralT, typename... Args>
class AbstractFactory : virtual public omm::Typed
{
protected:
  using creator_type = std::unique_ptr<GeneralT> (*)(Args&&...);
  using creator_map_type = std::map<Key, creator_type>;
  using clone_type = std::unique_ptr<GeneralT> (*)(const GeneralT&);
  using clone_map_type = std::map<Key, clone_type>;

private:
  template<typename SpecialT> static std::unique_ptr<GeneralT> make_(Args&&... args)
  {
    return std::make_unique<SpecialT>(std::forward<Args>(args)...);
  }

  template<typename SpecialT> static std::unique_ptr<GeneralT> clone_(const GeneralT& original)
  {
    if constexpr (std::is_same_v<decltype(SpecialT::TYPE), QString()>) {
      assert(original.type() == SpecialT::TYPE());
    } else {
      assert(original.type() == SpecialT::TYPE);
    }

    return std::make_unique<SpecialT>(static_cast<const SpecialT&>(original));
  }

public:
  class InvalidKeyError : public std::runtime_error
  {
  public:
    InvalidKeyError(const QString& what) : std::runtime_error(what.toStdString())
    {
    }
  };

  using factory_item_type = GeneralT;
  template<typename SpecialT> static void register_type(Key key)
  {
    m_creator_map[key] = &make_<SpecialT>;
    if constexpr (enable_clone) {
      m_clone_map[key] = &clone_<SpecialT>;
    }
  }

  template<typename... Args_> static std::unique_ptr<GeneralT> make(const Key& key, Args_&&... args)
  {
    const auto it = m_creator_map.find(key);
    if (it == m_creator_map.end()) {
      throw InvalidKeyError(QObject::tr("No such item: %2").arg(key));
    }
    // TODO shouldn't it be `Args_` instead of `Args`?
    return it->second(std::forward<Args>(args)...);
  }

  static std::unique_ptr<GeneralT> make(const Key& key)
  {
    static_assert(sizeof...(Args) == 0, "Expected zero arguments.");
    const auto it = m_creator_map.find(key);
    if (it == m_creator_map.end()) {
      throw InvalidKeyError(QObject::tr("No such item: %2").arg(key));
    }
    return it->second();
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
    std::transform(m_creator_map.begin(),
                   m_creator_map.end(),
                   std::inserter(keys, keys.end()),
                   [](const auto& pair) { return pair.first; });
    return keys;
  }

private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static creator_map_type m_creator_map;

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static clone_map_type m_clone_map;
};

template<typename Key, bool enable_clone, typename GeneralT, typename... Args>
typename AbstractFactory<Key, enable_clone, GeneralT, Args...>::creator_map_type
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    AbstractFactory<Key, enable_clone, GeneralT, Args...>::m_creator_map;

template<typename Key, bool enable_clone, typename GeneralT, typename... Args>
typename AbstractFactory<Key, enable_clone, GeneralT, Args...>::clone_map_type
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
    AbstractFactory<Key, enable_clone, GeneralT, Args...>::m_clone_map;
