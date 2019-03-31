#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <cassert>
#include <set>
#include <algorithm>
#include <type_traits>
#include <variant>
#include <typeinfo>
#include <utility>
#include <sstream>
#include <list>

/*
 * passes ownership of `object` to `consumer` and returns a reference to `object`
 */
template<typename T, typename F> T& transfer(std::unique_ptr<T> object, F consumer)
{
  T& ref = *object;
  consumer(std::move(object));
  return ref;
}

template<typename T, template<typename...> class Container> void reserve(Container<T>&, size_t)
{
  // for most containers, reserving is a no op.
}

template<typename T> void reserve(std::vector<T>& c, size_t n)
{
  c.reserve(n);
}

static constexpr struct identity_t
{
  template<typename T>
  constexpr decltype(auto) operator()(T&& t) const noexcept { return std::forward<T>(t); }
} identity {};

/**
 * @brief makes a container similar to `Container<S>` but with `value_type T`.
 */
template<typename T, template<typename...> class Container, typename S>
auto make_container(const Container<S>&) { return Container<T>(); }

template<typename T, typename ContainerS, typename F>
auto transform(ContainerS&& ss, F&& mapper)
{
  auto ts = make_container<T>(ss);
  reserve(ts, ss.size());
  std::transform( std::begin(ss), std::end(ss),
                  std::inserter(ts, std::end(ts)), std::forward<F>(mapper) );
  return ts;
}

template<typename T, typename ContainerS>
auto transform(ContainerS&& ss)
{
  return ::transform<T, ContainerS>(std::forward<ContainerS>(ss), ::identity);
}

template<typename T, template<typename...> class ContainerT, typename ContainerS, typename F>
auto transform(ContainerS&& ss, F&& mapper)
{
  ContainerT<T> ts;
  reserve(ts, ss.size());
  std::transform( std::begin(ss), std::end(ss),
                  std::inserter(ts, std::end(ts)), std::forward<F>(mapper) );
  return ts;
}

template<typename T, template<typename...> class ContainerT, typename ContainerS>
auto transform(ContainerS&& ss)
{
  return ::transform<T, ContainerT>(std::forward<ContainerS>(ss), ::identity);
}

template<typename Ts, typename F=identity_t, typename EqualT=std::equal_to<>>
auto is_uniform( const Ts& container,
                 const F& mapper=::identity,
                 const EqualT& equal_to = std::equal_to<>() )
{
  if (container.size() == 0) { return true; }
  decltype(auto) first = mapper(*std::begin(container));
  for (const auto& v : container) {
    if (!equal_to(mapper(v), first)) { return false; }
  }
  return true;
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

template<typename Ts, typename S> bool contains(const Ts& set, S&& key)
{
  if constexpr (std::is_pointer_v<S> || std::is_reference_v<S>) {
    return std::find(set.begin(), set.end(), const_cast<std::remove_const_t<S>>(key)) != set.end();
  } else {
    return std::find(set.begin(), set.end(), key) != set.end();
  }
}

template<typename T> struct is_unique_ptr : std::true_type {};
template<typename... T> struct is_unique_ptr<std::unique_ptr<T...>> : std::true_type {};


template<typename T, template<typename...> class ContainerT>
ContainerT<std::unique_ptr<T>> copy(const ContainerT<std::unique_ptr<T>>& other)
{
  return ::transform<std::unique_ptr<T>>(other, [](const auto& i) { return i->clone(); });
}

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

// similar to std::conditional, but with values rather than types
template<bool condition, typename T1, typename T2>
constexpr decltype(auto) conditional(T1&& t1, T2&& t2) noexcept
{
  if constexpr (condition) {
    return std::forward<T1>(t1);
  } else {
    return std::forward<T2>(t2);
  }
}

template<std::size_t i=0, typename variant_type>
void print_variant_value(std::ostream& ostream, const variant_type& variant)
{
  if (i == variant.index()) {
    ostream << std::get<i>(variant);
    return;
  }

  if constexpr (i+1 < std::variant_size_v<variant_type>) {
    print_variant_value<i+1, variant_type>(ostream, variant);
  }
}

template<typename T, typename S> T type_cast(S* s)
{
  if (s->type() == std::decay_t<std::remove_pointer_t<T>>::TYPE) {
    return static_cast<T>(s);
  } else {
    return nullptr;
  }
}

template<typename T, typename S> T& type_cast(S& s)
{
  auto* t = type_cast<std::add_pointer_t<T>>(&s);
  if (t == nullptr) {
    throw std::bad_cast();
  }
  return *t;
}

template<typename T, template<typename...> class Container, typename S>
Container<T> type_cast(const Container<S>& apos)
{
  const auto casted = ::transform<T>(apos, [](S apo) { return type_cast<T>(apo); });
  return ::filter_if(casted, ::is_not_null);
}

template<typename T> struct dereference
{
  decltype(auto) operator()(std::add_pointer_t<T> val) const { return *val; }
};

template<typename T> std::ostream& operator<<(std::ostream& ostream, const std::vector<T>& vs)
{
  ostream << "[ ";
  if (vs.size() > 0) {
    ostream << vs[0];
    for (std::size_t i = 1; i < vs.size(); ++i) { ostream << ", " << vs[i]; }
  }
  ostream << " ]";
  return ostream;
}

template<typename T> std::ostream& operator<<(std::ostream& ostream, const std::list<T>& vs)
{
  ostream << std::vector(vs.begin(), vs.end());
  return ostream;
}
