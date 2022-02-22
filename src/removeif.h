#pragma once

#include <vector>
#include <set>

template<typename T> class QList;

namespace util
{

namespace remove_if_detail
{

struct Predicate
{
  bool operator()(const auto&) const noexcept;
};

template<typename Container> concept erase_if_compatible = requires(Container c)
{
  { std::erase_if(c, Predicate{}) };
};

}  // namespace remove_if_detail

template<remove_if_detail::erase_if_compatible Ts, typename F>
auto remove_if(Ts ts, const F& predicate)
{
  std::erase_if(ts, predicate);
  return ts;
}

template<typename T, typename F>
auto remove_if(QList<T> ts, const F& predicate)
{
  ts.erase(std::remove_if(ts.begin(), ts.end(), predicate), ts.end());
  return ts;
}

}  // namespace util
