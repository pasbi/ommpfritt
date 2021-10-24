#pragma once

#include "geometry/point.h"

namespace omm
{
class Scene;
class Path;

template<typename PathRef> struct PathIteratorBase {
  using value_type = Point;
  static_assert(std::is_reference_v<PathRef>);
  static constexpr bool Const = std::is_const_v<std::remove_reference_t<PathRef>>;
  using reference = std::conditional_t<Const, const value_type&, value_type&>;
  using pointer = std::conditional_t<Const, const value_type*, value_type*>;
  using difference_type = int;
  using iterator_category = std::forward_iterator_tag;

  PathIteratorBase(PathRef path, std::size_t segment, std::size_t point);

  std::add_pointer_t<std::remove_reference_t<PathRef>> path;
  std::size_t segment;
  std::size_t point;

  bool operator<(const PathIteratorBase& other) const;
  bool operator>(const PathIteratorBase& other) const;
  bool operator==(const PathIteratorBase& other) const;
  bool operator!=(const PathIteratorBase& other) const;

  [[nodiscard]] bool is_end() const;
  reference operator*() const;
  pointer operator->() const;

  PathIteratorBase& operator++();
};

using PathIterator = PathIteratorBase<Path&>;
using PathConstIterator = PathIteratorBase<const Path&>;

}  // namespace omr
