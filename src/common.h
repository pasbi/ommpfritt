#pragma once

#include "transparentset.h"
#include "transform.h"
#include <QString>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>
#include <deque>

namespace omm
{

enum class Kind {
  None = 0x0,
  Tag = 0x1,
  Style = 0x2,
  Object = 0x4,
  Tool = 0x8,
  Node = 0x10,
  Item = Tag | Style | Object,
  All = Item | Tool | Node
};

enum class Flag {
  None = 0x0,
  Convertible = 0x1,
  HasScript = 0x2,
  IsView = 0x8,
  HasPythonNodes = 0x10,
  HasGLSLNodes = 0x20,
  HasNodes = HasPythonNodes | HasGLSLNodes,
  HasPython = HasPythonNodes | HasScript
};

enum class InterpolationMode { Linear, Smooth, Bezier };
enum class HandleStatus { Hovered, Active, Inactive };
enum class SceneMode { Object, Vertex, Face };

}  // namespace omm

enum class Stream { stdout_, stderr_ };

template<typename Ts, typename F> bool any_of(const Ts& ts, const F& f)
{
  return std::any_of(begin(ts), end(ts), f);
}

template<typename T, template<typename...> class ContainerT>
T& insert(ContainerT<std::unique_ptr<T>>& container, std::unique_ptr<T> obj, std::size_t pos)
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
  const auto is_obj = [&obj](const std::unique_ptr<T>& a) { return a.get() == &obj; };
  const auto it = std::find_if(std::begin(container), std::end(container), is_obj);
  assert(it != std::end(container));
  std::unique_ptr<T> uptr = std::move(*it);
  container.erase(it);
  return uptr;
}

template<typename SetA, typename SetB = SetA> decltype(auto) merge(SetA&& a, SetB&& b)
{
  a.insert(b.begin(), b.end());
  return std::forward<SetA>(a);
}

template<typename SetA, typename SetB = SetA> SetA merge(const SetA& a, SetB&& b)
{
  auto copy = a;
  copy.insert(b.begin(), b.end());
  return copy;
}

template<typename SetA, typename SetB, typename... Sets>
SetA merge(SetA&& a, SetB&& b, Sets&&... sets)
{
  return merge(merge(a, b), std::forward<Sets>(sets)...);
}

template<typename Container, typename S>
bool contains(const Container& set, const S& key)
    requires requires { { *begin(set) == key } -> std::same_as<bool>; }
{
  return std::find_if(begin(set), end(set), [&key](const auto& v) { return v == key; }) != end(set);
}

template<typename S, typename... Ts> bool contains(const std::map<Ts...>& map, S&& key)
{
  if constexpr (std::is_pointer_v<S> || std::is_reference_v<S>) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return map.find(const_cast<std::remove_const_t<S>>(key)) != map.end();
  } else {
    return map.find(key) != map.end();
  }
}

template<typename T> struct is_unique_ptr : std::false_type {
};
template<typename... T> struct is_unique_ptr<std::unique_ptr<T...>> : std::true_type {
};

template<typename C> struct HasCloneMethod {
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
    return util::transform(other, [](const auto& i) { return i->clone(); });
  } else {
    return util::transform(other, [](const auto& i) { return std::make_unique<T>(*i); });
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

template<typename T, typename S> decltype(auto) type_cast(S&& s)
{
  if constexpr (std::is_pointer_v<std::decay_t<T>>) {
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
      return static_cast<T>(nullptr);
    }
  } else {
    static_assert(!std::is_pointer_v<T>);
    auto* t = type_cast<std::add_pointer_t<T>>(&s);
    if (t == nullptr) {
      throw std::bad_cast();
    }
    return *t;
  }
}

template<typename T, template<typename...> class Container, typename S>
Container<T> type_casts(const Container<S>& apos)
{
  auto casted = util::transform(apos, [](S apo) { return type_cast<T>(apo); });
  std::erase_if(casted, [](const auto* p) { return p == nullptr; });
  return casted;
}

template<typename S, typename T, typename... Ts> constexpr bool typelist_contains() noexcept
{
  if constexpr (std::is_same_v<S, T>) {
    return true;
  } else if constexpr (sizeof... (Ts) == 0) {
    return false;
  } else {
    return typelist_contains<S, Ts...>();
  }
}

template<typename K, typename V> std::set<K> get_keys(const std::map<K, V>& map)
{
  return util::transform<std::set>(map, [](const auto& key_value) {
    return key_value.first;
  });
}

template<typename R, typename Ts, typename P, typename F>
auto find_if(Ts&& items, P&& predicate, F&& f, R&& default_)
{
  for (auto&& t : items) {
    if (predicate(t)) {
      return f(t);
    }
  }
  return default_;
}

/**
 * @brief copy_unique_ptr sometimes it makes sense to copy a unique ptr, i.e., to create
 *  a new unique ptr with a copied content.
 *  Calls T's copy constructor.
 * @param up the unique_ptr to copy
 * @return a new unique pointer holding a copy of the old unique_ptr's content.
 */
template<typename T, typename... Args> std::unique_ptr<T> copy_unique_ptr(const std::unique_ptr<T>& up, Args&&... args)
{
  if (up) {
    return std::make_unique<T>(*up, std::forward<Args>(args)...);
  } else {
    return {};
  }
}

namespace omm
{

template<typename T> struct EnableBitMaskOperators : std::false_type { };

template<typename EnumT> requires EnableBitMaskOperators<EnumT>::value EnumT operator|(EnumT a, EnumT b)
{
  using underlying = std::underlying_type_t<EnumT>;
  return static_cast<EnumT>(static_cast<underlying>(a) | static_cast<underlying>(b));
}

template<typename EnumT> requires EnableBitMaskOperators<EnumT>::value EnumT operator&(EnumT a, EnumT b)
{
  using underlying = std::underlying_type_t<EnumT>;
  return static_cast<EnumT>(static_cast<underlying>(a) & static_cast<underlying>(b));
}

template<typename EnumT> requires EnableBitMaskOperators<EnumT>::value EnumT operator~(EnumT a)
{
  return static_cast<EnumT>(~static_cast<std::underlying_type_t<EnumT>>(a));
}

template<typename EnumT> requires EnableBitMaskOperators<EnumT>::value bool operator!(EnumT a)
{
  return !static_cast<std::underlying_type_t<EnumT>>(a);
}

template<typename EnumT> requires EnableBitMaskOperators<EnumT>::value EnumT& operator|=(EnumT& a, EnumT b)
{
  a = a | b;
  return a;
}

template<typename EnumT> requires EnableBitMaskOperators<EnumT>::value EnumT& operator&=(EnumT& a, EnumT b)
{
  a = a & b;
  return a;
}

template<typename Vertex> std::pair<bool, std::list<Vertex>>
topological_sort(std::set<Vertex> vertices, const std::function<std::set<Vertex>(Vertex)>& successors)
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
  return {has_cycles, sequence};
}

template<typename Vertex>
bool find_path(Vertex start,
               Vertex target,
               std::list<Vertex>& path,
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

constexpr double M_180_PI = 180.0 * M_1_PI;
constexpr double M_PI_180 = M_PI / 180.0;

template<typename Cs>
QString join(const Cs& ts, const std::function<QString(const typename Cs::value_type&)>& to_string)
{
  if (ts.empty()) {
    return {};
  } else {
    QString s{"["};
    auto it = begin(ts);
    s += to_string(*it);
    std::advance(it, 1);
    for (; it != end(ts); std::advance(it, 1)) {
      s += ", " + to_string(*it);
    }
    s += "]";
    return s;
  }
}

/**
 * @brief find_coherent_ranges returns list of ranges such that cover every element in @param vs
 *  that @param evaluates true for.
 */
template<typename Vs, typename F> auto find_coherent_ranges(const Vs& vs, F&& f)
{
  struct Range
  {
    std::size_t start;
    std::size_t size;
    bool operator<(const Range& other) const noexcept { return start < other.start; }
  };

  std::deque<Range> ranges;
  bool in_selection = false;
  for (std::size_t i = 0; i < vs.size(); ++i) {
    if (f(vs[i])) {
      if (!in_selection) {
        ranges.push_back({i, 1});
      } else if (in_selection) {
        ranges.back().size += 1;
      }
      in_selection = true;
    } else {
      in_selection = false;
    }
  }
  return ranges;
}

template<typename T> auto python_like_mod(const T& dividend, const T& divisor)
{
  return std::fmod(std::fmod(dividend, divisor) + divisor, divisor);
}

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::Kind> : std::true_type { };
template<> struct omm::EnableBitMaskOperators<omm::Flag> : std::true_type { };
