#include <type_traits>
#include <utility>
#include "functional"


namespace util
{

namespace transform_detail
{
template<typename C> concept has_push_back = requires(C c)
{
  { c.push_back(std::declval<typename C::value_type>()) };
};

template<typename C> concept has_insert = requires(C c)
{
  { c.insert(std::declval<typename C::value_type>()) };
};

template<typename C> concept has_reserve = requires(const C c)
{
  { c.reserve(std::size_t{}) };
};

template<typename F, typename A> concept transformer = requires(F f)
{
  { std::invoke(f, std::declval<A>()) };
};

template<has_push_back C, typename T> void push_back(C& cs, T&& val)
{
  cs.push_back(std::forward<T>(val));
}

template<has_insert C, typename T> void push_back(C& cs, T&& val)
{
  cs.insert(std::forward<T>(val));
}

// Base implementation for non-associative containers
template<typename ValueType,
         template<typename...> typename ReturnContainer,
         typename Container,
         transformer<ValueType> F>
auto transform_helper(Container&& cs, const F& f = F{})
{
  using ReturnValueType = std::decay_t<decltype(std::invoke(f, std::declval<ValueType>()))>;
  ReturnContainer<ReturnValueType> transformed;
  if constexpr (has_reserve<ReturnContainer<ReturnValueType>>) {
    transformed.reserve(cs.size());
  }
  for (auto&& c : cs) {
    transform_detail::push_back(transformed, std::invoke(f, std::move(c)));
  }
  return transformed;
}

// Base implementation for associative containers
template<typename ValueType,
         template<typename...> typename ReturnContainer,
         typename... ContainerArguments,
         template<typename...> typename Container,
         transformer<ValueType> F,
         typename ConcreteContainer = Container<std::decay_t<typename ValueType::first_type>,
                                                std::decay_t<typename ValueType::second_type>,
                                                ContainerArguments...>>
auto transform_helper(const ConcreteContainer& cs, const F& f = F{})
{
  return transform_helper<ValueType, ReturnContainer, ConcreteContainer, F>(cs, f);
}

// Base implementation for associative containers that need to be moved
template<typename ValueType,
         template<typename...> typename ReturnContainer,
         typename... ContainerArguments,
         template<typename...> typename Container,
         transformer<ValueType> F,
         typename ConcreteContainer = Container<std::decay_t<typename ValueType::first_type>,
                                                std::decay_t<typename ValueType::second_type>,
                                                ContainerArguments...>>
auto transform_helper(ConcreteContainer&& cs, const F& f = F{})
{
  return transform_helper<ValueType, ReturnContainer, ConcreteContainer, F>(std::forward<decltype(cs)>(cs), f);
}

}  // namespace transform_detail


// overload for arrays
template<typename ValueType,
         template<typename...> typename ReturnContainer,
         template<typename, std::size_t> typename Container,
         transform_detail::transformer<ValueType> F, std::size_t N>
auto transform(const Container<ValueType, N>& cs, const F& f = F{})
{
  using transform_detail::transform_helper;
  return transform_helper<ValueType, ReturnContainer, Container<ValueType, N>, F>(cs, f);
}

// overload for arrays that need to be moved
template<typename ValueType,
         template<typename...> typename ReturnContainer,
         template<typename, std::size_t> typename Container,
         transform_detail::transformer<ValueType> F, std::size_t N>
auto transform(Container<ValueType, N>&& cs, const F& f = F{})
{
  using transform_detail::transform_helper;
  return transform_helper<ValueType, ReturnContainer, Container<ValueType, N>, F>(cs, f);
}

// overload for explicit return value type
template<typename ReturnValueType, template<typename...> typename Container, typename... ContainerArgs>
auto transform(const Container<ContainerArgs...>& cs)
{
  using transform_detail::transform_helper;
  auto f = [](auto&& v) { return static_cast<ReturnValueType>(v); };
  return transform_helper<typename Container<ContainerArgs...>::value_type, Container>(cs, f);
}

// overload for explicit return value type (move)
template<typename ReturnValueType, template<typename...> typename Container, typename... ContainerArgs>
auto transform(Container<ContainerArgs...>&& cs)
{
  using transform_detail::transform_helper;
  auto f = [](auto&& v) { return static_cast<ReturnValueType>(v); };
  return transform_helper<typename Container<ContainerArgs...>::value_type, Container>(std::move(cs), f);
}

// overload for explicit return container type and functor
template<template<typename...> typename ReturnContainer,
         typename Container,
         transform_detail::transformer<typename std::decay_t<Container>::value_type> F>
auto transform(Container&& cs, const F& f)
{
  using transform_detail::transform_helper;
  using ValueType = typename std::decay_t<Container>::value_type;
  return transform_helper<ValueType, ReturnContainer>(std::forward<Container>(cs), f);
}

// overload for explicit return container type
template<template<typename...> typename ReturnContainer, typename Container>
auto transform(Container&& cs)
{
  using transform_detail::transform_helper;
  constexpr auto f = [](auto&& v) { return std::forward<decltype(v)>(v); };
  using value_type = typename std::decay_t<Container>::value_type;
  return transform_helper<value_type, ReturnContainer>(std::forward<Container>(cs), f);
}

// overload for explicit return value type and return container type
template<typename ReturnValueType, template<typename ...> typename ReturnContainer, typename Container>
auto transform(Container&& cs)
{
  using transform_detail::transform_helper;
  constexpr auto f = [](auto&& v) { return static_cast<ReturnValueType>(v); };
  using value_type = typename std::decay_t<Container>::value_type;
  return transform_helper<value_type, ReturnContainer>(std::forward<Container>(cs), f);
}

// overload for implicit return container type and functor
template<template<typename...> typename Container,
         typename... ContainerArgs,
         transform_detail::transformer<typename std::decay_t<Container<ContainerArgs...>>::value_type> F>
auto transform(const Container<ContainerArgs...>& cs, const F& f)
{
  using transform_detail::transform_helper;
  return transform_helper<typename Container<ContainerArgs...>::value_type, Container>(cs, f);
}

// overload for implicit return container type and functor (move)
template<template<typename...> typename Container, typename... ContainerArgs, typename F>
auto transform(Container<ContainerArgs...>&& cs, const F& f)
{
  using transform_detail::transform_helper;
  return transform_helper<typename Container<ContainerArgs...>::value_type, Container>(std::move(cs), f);
}

}  // namespace omm::util
