#pragma once

#include <glog/logging.h>
#include <set>
#include <memory>

namespace omm
{

class Scene;

template<typename T> class Structure
{
public:
  using item_type = T;
  Structure(Scene& scene);
  virtual ~Structure();

  virtual std::set<T*> items() const = 0;
  virtual size_t position(const T& item) const = 0;
  virtual const T* predecessor(const T& sibling) const = 0;
  size_t insert_position(const T* predecessor) const;

  virtual T& insert(std::unique_ptr<T> t) = 0;
  virtual std::unique_ptr<T> remove(T& t) = 0;

  Scene& scene;

  virtual void invalidate() = 0;

protected:
  void invalidate_recursive();

private:
  // we don't want to assign copy or move
  const Structure<T>& operator=(const Structure<T>&) = delete;
  const Structure<T>& operator=(Structure<T>&&) = delete;
  Structure<T>(const Structure<T>&) = delete;
  Structure<T>(Structure<T>&&) = delete;
};

}  // namespace omm
