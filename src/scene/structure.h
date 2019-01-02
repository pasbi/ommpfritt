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
  Structure(Scene* scene = nullptr);
  Structure(Structure&& other) = default;
  virtual ~Structure() = default;

  void set_scene(Scene* scene);

  virtual std::set<T*> items() const = 0;
  virtual size_t position(const T& item) const = 0;
  virtual const T* predecessor(const T& sibling) const = 0;
  size_t insert_position(const T* predecessor) const;

  virtual std::unique_ptr<T> remove(T& t) = 0;

  virtual void invalidate() = 0;

protected:
  void invalidate_recursive();

private:
  // we don't want to assign copy or move
  const Structure<T>& operator=(const Structure<T>&) = delete;
  const Structure<T>& operator=(Structure<T>&&) = delete;
  Structure<T>(const Structure<T>&) = delete;

  // TODO actually Structure should not have a pointer to scene.
  Scene* m_scene;
};

}  // namespace omm
