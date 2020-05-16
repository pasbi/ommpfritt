#pragma once

#include <QString>
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
#include <map>

namespace omm
{

enum class Kind { None = 0x0,
                  Tag = 0x1, Style = 0x2, Object = 0x4, Tool = 0x8, Node = 0x10,
                  Item = Tag | Style | Object, All = Item | Tool | Node };

enum class Flag { None = 0x0,
                  Convertible = 0x1, HasScript = 0x2, IsPathLike = 0x4, IsView = 0x8,
                  HasPythonNodes = 0x10, HasGLSLNodes = 0x20,
                  HasNodes = HasPythonNodes | HasGLSLNodes,
                  HasPython = HasPythonNodes | HasScript };

enum class HandleStatus { Hovered, Active, Inactive };

}  // namespace omm

enum class Stream { stdout_, stderr_ };

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
auto transform(ContainerS&& ss, F&& mapper = ::identity)
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
  return uptr;
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

template<template<typename...> typename Container, typename S, typename... Ts>
bool contains(const Container<Ts...>& set, S&& key)
{
  if constexpr (std::is_pointer_v<S> || std::is_reference_v<S>) {
    return std::find(set.begin(), set.end(), const_cast<std::remove_const_t<S>>(key)) != set.end();
  } else {
    return std::find(set.begin(), set.end(), key) != set.end();
  }
}

template<typename S, typename... Ts> bool contains(const std::map<Ts...>& map, S&& key)
{
  if constexpr (std::is_pointer_v<S> || std::is_reference_v<S>) {
    return map.find(const_cast<std::remove_const_t<S>>(key)) != map.end();
  } else {
    return map.find(key) != map.end();
  }
}

template<typename T> struct is_unique_ptr : std::true_type {};
template<typename... T> struct is_unique_ptr<std::unique_ptr<T...>> : std::true_type {};

template<typename C>
struct HasCloneMethod {
private:
  template<typename T> using X = decltype(std::declval<T>().clone());
  template<typename T> using U = typename std::is_same<X<T>, std::unique_ptr<T>>::type;
  template<typename T> static constexpr auto check(T*) -> U<T>;
  template<typename> static constexpr std::false_type check(...);
  using type = decltype(check<C>(0));

public:
  static constexpr bool value = type::value;
};


template<typename T, template<typename...> class ContainerT>
ContainerT<std::unique_ptr<T>> copy(const ContainerT<std::unique_ptr<T>>& other)
{
  if constexpr (HasCloneMethod<T>::value) {
    return ::transform<std::unique_ptr<T>>(other, [](const auto& i) { return i->clone(); });
  } else {
    return ::transform<std::unique_ptr<T>>(other, [](const auto& i) {
      return std::make_unique<T>(*i);
    });
  }

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
    if constexpr (std::is_same_v<std::variant_alternative_t<i, variant_type>, QString>) {
      ostream << std::get<i>(variant).toStdString();
    } else {
      ostream << std::get<i>(variant);
    }
    return;
  }

  if constexpr (i+1 < std::variant_size_v<variant_type>) {
    print_variant_value<i+1, variant_type>(ostream, variant);
  }
}

template<typename T, typename S> T type_cast(S* s)
{
  const auto type = []() {
    using DecayedT = std::decay_t<std::remove_pointer_t<T>>;
    if constexpr (std::is_same_v<decltype(DecayedT::TYPE), QString()>) {
      return DecayedT::TYPE();
    } else {
      return DecayedT::TYPE;
    }
  }();
  if (s != nullptr && s->type() == type) {
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

template<typename T> std::ostream& operator<<(std::ostream& ostream, const std::set<T>& vs)
{
  ostream << std::vector(vs.begin(), vs.end());
  return ostream;
}

template<typename K, typename V> std::set<K> get_keys(const std::map<K, V>& map)
{
  std::set<K> keys;
  for (auto&& [k, v] : map) {
    (void) v;
    keys.insert(k);
  }
  return keys;
}

template<typename Map1, typename Map2>
bool same_keys(const Map1& m1, const Map2& m2)
{
  if (m1.size() != m2.size()) {
    return false;
  } else {
    return std::equal(m1.begin(), m1.end(), m2.begin(),  [](const auto& a, const auto& b) {
      return a.first == b.first;
    });
  }
}

template<typename R, typename Ts, typename P, typename F> auto
find_if(Ts&& items, P&& predicate, F&& f, R&& default_)
{
  for (auto&& t : items) {
    if (predicate(t)) {
      return f(t);
    }
  }
  return default_;
}

namespace omm
{

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

template<typename EnumT>
std::enable_if_t<EnableBitMaskOperators<EnumT>::value, EnumT&> operator|=(EnumT& a, EnumT b)
{
  a = a | b;
  return a;
}

template<typename EnumT>
std::enable_if_t<EnableBitMaskOperators<EnumT>::value, EnumT&> operator&=(EnumT& a, EnumT b)
{
  a = a & b;
  return a;
}

template<typename Vertex> std::pair<bool, std::list<Vertex>>
topological_sort(std::set<Vertex> vertices,
                 const std::function<std::set<Vertex>(Vertex)>& successors)
{
  // Kahn's algorithm

  std::list<Vertex> sequence;
  std::set<Vertex> next_vertices = vertices;
  std::map<Vertex, std::set<Vertex>> edges;
  for (Vertex vertex : vertices) {
    for (Vertex successor : successors(vertex)) {
      edges[successor].insert(vertex);
      next_vertices.erase(successor);
    }
  }

  while (!next_vertices.empty()) {
    auto it = next_vertices.begin();
    Vertex vertex = *it;
    next_vertices.erase(it);
    sequence.push_back(vertex);
    for (Vertex successor : successors(vertex)) {
      edges[successor].erase(vertex);
      if (edges[successor].empty()) {
        edges.erase(successor);
        next_vertices.insert(successor);
      }
    }
  }

  const bool has_cycles = !edges.empty();
  return { has_cycles, sequence };
}

template<typename Vertex> bool
find_path(Vertex start, Vertex target, std::list<Vertex>& path,
          const std::function<std::set<Vertex>(Vertex)>& get_successors)
{
  if (start == target) {
    return true;
  } else {
    for (Vertex successor : get_successors(start)) {
      if (find_path(successor, target, path, get_successors)) {
        path.push_back(successor);
        return true;
      }
    }
    return false;
  }
}

enum class Space { Viewport, Scene };

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::Kind> : std::true_type {};
template<> struct omm::EnableBitMaskOperators<omm::Flag> : std::true_type {};
