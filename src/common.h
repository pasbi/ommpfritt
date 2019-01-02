#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <cassert>
#include <set>
#include <algorithm>
#include <type_traits>

/*
 * passes ownership of `object` to `consumer` and returns a reference to `object`
 */
template<typename T, typename F> T& transfer(std::unique_ptr<T> object, F consumer)
{
  T& ref = *object;
  consumer(std::move(object));
  return ref;
}

template<typename T, template<typename...> class Container> void reserve(Container<T>& c, size_t)
{
  // for most containers, reserving is a no op.
}

template<typename T> void reserve(std::vector<T>& c, size_t n)
{
  c.reserve(n);
}

template<typename T, typename S, template<typename...> class Container, typename F>
auto transform(const Container<S>& ss, F&& mapper)
{
  Container<T> ts;
  reserve(ts, ss.size());
  std::transform( std::begin(ss), std::end(ss),
                  std::inserter(ts, std::end(ts)), std::forward<F>(mapper) );
  return ts;
}

template< typename T, template<typename...> class ContainerT,
          typename S, template<typename...> class ContainerS, typename F >
auto transform(const ContainerS<S>& ss, F&& mapper)
{
  ContainerT<T> ts;
  reserve(ts, ss.size());
  std::transform( std::begin(ss), std::end(ss),
                  std::inserter(ts, std::end(ts)), std::forward<F>(mapper) );
  return ts;
}

template<typename Ts, typename F>
auto is_uniform(const Ts& container, F&& mapper)
{
  const auto& first = mapper(*std::begin(container));
  for (const auto& v : container) {
    if (mapper(v) != first) {
      return false;
    }
  }
  return true;
}

template<typename Ts>
auto is_uniform(const Ts& container)
{
  return is_uniform<Ts>(container, [](const auto& f) { return f; });
}

template<typename T, template<typename...> class ContainerT>
T& insert(ContainerT<std::unique_ptr<T>>& container, std::unique_ptr<T> obj, size_t pos)
{
  auto pos_it = container.begin();
  std::advance(pos_it, pos);

  T& ref = *obj;
  assert(obj.get() != nullptr);
  container.insert(pos_it, std::move(obj));
  return ref;
}

template<typename T, template<typename...> class ContainerT>
std::unique_ptr<T> extract(ContainerT<std::unique_ptr<T>>& container, const T& obj)
{
  const auto is_obj = [&obj] (const std::unique_ptr<T>& a) { return a.get() == &obj; };
  const auto it = std::find_if(std::begin(container), std::end(container), is_obj);
  assert(it != std::end(container));
  std::unique_ptr<T> uptr = std::move(*it);
  container.erase(it);
  return std::move(uptr);
}

template<typename Ts, typename Predicate>
Ts filter_if(const Ts& ts, const Predicate& p)
{
  Ts filtered;
  std::copy_if(ts.begin(), ts.end(), std::inserter(filtered, filtered.end()), p);
  return filtered;
}

bool is_not_null(const void* p);

template<typename SetA, typename SetB=SetA> SetA merge(SetA&& a, SetB&& b)
{
  a.insert(b.begin(), b.end());
  return a;
}

template<typename SetA, typename SetB, typename... Sets>
SetA merge(SetA&& a, SetB&& b, Sets&&... sets)
{
  return merge(merge(a, b), std::forward<Sets>(sets)...);
}

template<typename T, typename Predicate> void erase_if(std::set<T>& ts, const Predicate& p)
{
  // TODO replace with std::erase_if once we have c++2a.
  // See https://stackoverflow.com/q/53791992/4248972
  for (auto it = ts.begin(); it != ts.end(); ++it) {
    if (p(*it)) {
      ts.erase(it);
    }
  }
}

template<typename T, typename Predicate> void erase_if(std::vector<T>& ts, const Predicate& p)
{
  // TODO replace with std::erase_if once we have c++2a
  ts.erase(std::remove_if(ts.begin(), ts.end(), p), ts.end());
}

template<typename T, typename S> bool contains(const std::set<T*>& set, S const* const key)
{
  return set.count(const_cast<S*>(key)) > 0;
}

template<typename T, typename S> bool contains(const std::set<T>& set, const S& key)
{
  return set.count(const_cast<S&>(key)) > 0;
}

template<typename T> struct is_unique_ptr : std::true_type {};
template<typename... T> struct is_unique_ptr<std::unique_ptr<T...>> : std::true_type {};


template<typename T, template<typename...> class ContainerT>
ContainerT<std::unique_ptr<T>> copy(const ContainerT<std::unique_ptr<T>>& other)
{
  return ::transform<std::unique_ptr<T>>(other, [](const auto& i) { return i->clone(); });
}

static constexpr struct identity_t
{
  template<typename T>
  constexpr decltype(auto) operator()(T&& t) const noexcept { return std::forward<T>(t); }
} identity {};

template<typename T> struct EnableBitMaskOperators : std::false_type {};

template<typename EnumT>
std::enable_if_t<EnableBitMaskOperators<EnumT>::value, EnumT> operator|(EnumT a, EnumT b)
{
  using underlying = std::underlying_type_t<EnumT>;
  return static_cast<EnumT>(static_cast<underlying>(a) | static_cast<underlying>(b));
}

template<typename EnumT>
std::enable_if_t<EnableBitMaskOperators<EnumT>::value, EnumT> operator&(EnumT a, EnumT b)
{
  using underlying = std::underlying_type_t<EnumT>;
  return static_cast<EnumT>(static_cast<underlying>(a) & static_cast<underlying>(b));
}

template<typename EnumT>
std::enable_if_t<EnableBitMaskOperators<EnumT>::value, EnumT> operator~(EnumT a)
{
  return static_cast<EnumT>(~static_cast<std::underlying_type_t<EnumT>>(a));
}

template<typename EnumT>
std::enable_if_t<EnableBitMaskOperators<EnumT>::value, bool> operator!(EnumT a)
{
  return !static_cast<std::underlying_type_t<EnumT>>(a);
}

template<typename T, template<typename...> class ContainerT>
ContainerT<T> intersect(const ContainerT<T>& a, const ContainerT<T>& b)
{
  ContainerT<T> intersection;
  auto inserter = std::inserter(intersection, intersection.end());
  std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), inserter);
  return intersection;
}
