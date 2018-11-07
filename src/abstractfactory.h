#pragma once

#include <map>
#include <memory>

template<typename T>
class get_first_var_template
{
public:
  using type = T;
};

template<typename Key, typename GeneralT, typename... Args>
class AbstractFactory
{
protected:
  using creator_type = std::unique_ptr<GeneralT>(*)(Args&&... args);
  using creator_map_type = std::map<Key, creator_type>;
  // static_assert(get_first_var_template<Args...>::type::X, "X");


private:
  template<typename SpecialT>
  static std::unique_ptr<GeneralT> make_unique(Args&&... args)
  {
    return std::make_unique<SpecialT>(std::forward<Args>(args)...);
  }

public:
  template<typename SpecialT> static void register_type(Key key)
  {
    auto pair = std::pair<Key, creator_type>(key, &make_unique<SpecialT>);
    m_creator_map.insert(pair);
  }

  template<typename... Args_>
  static std::unique_ptr<GeneralT> make(const Key& key, Args_&&... args)
  {
    constexpr bool template_argument_is_valid = std::is_same< std::decay_t<Args_>...,
                                                              std::decay_t<Args >... >::value;
    static_assert(template_argument_is_valid, "Args_ and Args must be same.");
    return m_creator_map.at(key)(std::forward<Args>(args)...);
  }

  static std::unique_ptr<GeneralT> make(const Key& key)
  {
    static_assert(sizeof...(Args) == 0, "Expected zero arguments.");
    return m_creator_map.at(key)();
  }

public:
  virtual std::string type() const = 0;

private:
  static creator_map_type m_creator_map;
};

template<typename Key, typename GeneralT, typename... Args>
typename AbstractFactory<Key, GeneralT, Args...>::creator_map_type
AbstractFactory<Key, GeneralT, Args...>::m_creator_map;
