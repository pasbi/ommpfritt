#pragma once

#include <functional>
#include <memory>
#include <vector>

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

template<typename T, template<typename...> class ContainerT, typename F>
auto is_uniform(const ContainerT<T>& container, F&& mapper = [](const auto& f) { return f; })
{
  const auto& first = mapper(*std::begin(container));
  for (const auto& v : container) {
    if (mapper(v) != first) {
      return false;
    }
  }
  return true;
}

namespace omm
{

class Object;
using ObjectRef = std::reference_wrapper<Object>;
using ObjectRefs = std::vector<ObjectRef>;

}  // namespace omm
