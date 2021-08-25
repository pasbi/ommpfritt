#pragma once

#include <memory>
#include <set>

namespace omm
{
class Scene;

template<typename T> class Structure
{
public:
  using item_type = T;
  Structure() = default;
  Structure(Structure&& other) noexcept = default;
  virtual ~Structure() = default;

  virtual std::set<T*> items() const = 0;
  virtual std::unique_ptr<T> remove(T& t) = 0;
  virtual std::size_t position(const T& item) const = 0;
  virtual const T* predecessor(const T& sibling) const = 0;
  T* predecessor(T& sibling) const;
  virtual std::size_t insert_position(const T* predecessor) const = 0;

  // we don't want to assign copy or move
  const Structure<T>& operator=(const Structure<T>&) = delete;
  const Structure<T>& operator=(Structure<T>&&) = delete;
  Structure(const Structure<T>&) = delete;
};

}  // namespace omm
