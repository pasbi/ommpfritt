#include <type_traits>
#include <utility>


namespace util
{

template<typename C> concept has_push_back = requires(C c)
{
  { c.push_back(std::declval<typename C::value_type>()) };
};

template<typename C> concept has_insert = requires(C c)
{
  { c.insert(std::declval<typename C::value_type>()) };
};

template<typename C> concept has_reserve = requires(C c)
{
  { c.reserve(std::size_t{}) };
};

template<has_push_back C, typename T> void push_back(C& cs, T&& val)
{
  cs.push_back(std::forward<T>(val));
}

template<has_insert C, typename T> void push_back(C& cs, T&& val)
{
  cs.insert(std::forward<T>(val));
}

// Base overload for non-associative containers
template<typename ValueType,
         template<typename...> typename ReturnContainer,
         typename... ContainerArguments,
         template<typename...> typename Container,
         typename F>
auto transform(const Container<ValueType, ContainerArguments...>& cs, const F& f = F{})
{
  using ReturnValueType = decltype(f(std::declval<ValueType>()));
  ReturnContainer<ReturnValueType> transformed;
  if constexpr (has_reserve<ReturnContainer<ReturnValueType>>) {
    transformed.reserve(cs.size());
  }
  for (const auto& c : cs) {
    push_back(transformed, f(c));
  }
  return transformed;
}

// Base overload for associative containers
template<typename ValueType,
         template<typename...> typename ReturnContainer,
         typename... ContainerArguments,
         template<typename...> typename Container,
         typename F>
auto transform(const Container<std::decay_t<typename ValueType::first_type>, std::decay_t<typename ValueType::second_type>, ContainerArguments...>& cs, const F& f = F{})
{
  using ReturnValueType = decltype(f(std::declval<ValueType>()));
  ReturnContainer<ReturnValueType> transformed;
  if constexpr (has_reserve<ReturnContainer<ReturnValueType>>) {
    transformed.reserve(cs.size());
  }
  for (const auto& c : cs) {
    push_back(transformed, f(c));
  }
  return transformed;
}

// Base overload for array
template<typename ValueType,
         template<typename...> typename ReturnContainer,
         template<typename, std::size_t> typename Container,
         typename F, std::size_t N>
auto transform(const Container<ValueType, N>& cs, const F& f = F{})
{
  using ReturnValueType = decltype(f(std::declval<ValueType>()));
  ReturnContainer<ReturnValueType> transformed;
  if constexpr (has_reserve<ReturnContainer<ReturnValueType>>) {
    transformed.reserve(cs.size());
  }
  for (const auto& c : cs) {
    push_back(transformed, f(c));
  }
  return transformed;
}

// overload for explicit return value type
template<typename ReturnValueType,
         template<typename...> typename Container,
         typename... ContainerArgs>
auto transform(const Container<ContainerArgs...>& cs)
{
  auto f = [](auto&& v) { return static_cast<ReturnValueType>(v); };
  return transform<typename Container<ContainerArgs...>::value_type, Container>(cs, f);
}

// overload for explicit return container type and functor
template<template<typename...> typename ReturnContainer,
         typename Container,
         typename F>
auto transform(const Container& cs, const F& f)
{
  return transform<typename Container::value_type, ReturnContainer>(cs, f);
}

// overload for explicit return container type
template<template<typename...> typename ReturnContainer, typename Container>
auto transform(const Container& cs)
{
  constexpr auto f = [](auto&& v) { return v; };
  return transform<typename Container::value_type, ReturnContainer>(cs, f);
}

// overload for explicit return value type and return container type
template<typename ReturnValueType,
         template<typename ...> typename ReturnContainer,
         typename Container>
auto transform(const Container& cs)
{
  constexpr auto f = [](auto&& v) { return static_cast<ReturnValueType>(v); };
  return transform<typename Container::value_type, ReturnContainer>(cs, f);
}

// overload for explicit return container type and functor
template<template<typename...> typename Container,
         typename... ContainerArgs,
         typename F>
auto transform(const Container<ContainerArgs...>& cs, const F& f)
{
  return transform<typename Container<ContainerArgs...>::value_type, Container>(cs, f);
}

}  // namespace omm::util
