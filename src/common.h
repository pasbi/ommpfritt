#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <assert.h>
#include <algorithm>

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
std::unique_ptr<T> extract(ContainerT<std::unique_ptr<T>>& container, T& obj)
{
  const auto it = std::find_if( std::begin(container),
                                std::end(container),
                                [&obj](const std::unique_ptr<T>& a) {
    return a.get() == &obj;
  });
  assert(it != std::end(container));
  std::unique_ptr<T> uptr = std::move(*it);
  container.erase(it);
  return std::move(uptr);
}

namespace omm
{

class Object;  // TODO deprecated
using ObjectRef = std::reference_wrapper<Object>;  // TODO deprecated
using ObjectRefs = std::vector<ObjectRef>;  // TODO deprecated

}  // namespace omm
