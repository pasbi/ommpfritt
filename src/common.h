#pragma once

#include <functional>
#include <memory>

/*
 * passes ownership of `object` to `consumer` and returns a reference to `object`
 */
template<typename T, typename F> T& transfer(std::unique_ptr<T> object, F consumer)
{
  T& ref = *object;
  consumer(std::move(object));
  return ref;
}

template<typename T, typename S, template<typename...> class Container, typename F>
auto transform(const Container<S>& ss, F&& mapper)
{
  Container<T> ts;
  ts.reserve(ss.size());
  std::transform( std::begin(ss), std::end(ss),
                  std::inserter(ts, std::end(ts)), std::forward<F>(mapper) );
  return ts;
}

template< typename T, template<typename...> class ContainerT,
          typename S, template<typename...> class ContainerS, typename F >
auto transform(const ContainerS<S>& ss, F&& mapper)
{
  ContainerT<T> ts;
  ts.reserve(ss.size());
  std::transform( std::begin(ss), std::end(ss),
                  std::inserter(ts, std::end(ts)), std::forward<F>(mapper) );
  return ts;
}

namespace omm
{

class Object;
using ObjectRef = std::reference_wrapper<Object>;
using ObjectRefs = std::vector<ObjectRef>;

}  // namespace omm
