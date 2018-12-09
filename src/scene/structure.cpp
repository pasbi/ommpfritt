#include "scene/structure.h"
#include "common.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/scene.h"

namespace omm
{

template<typename T> Structure<T>::Structure(Scene& scene) : m_scene(scene) {}
template<typename T> Structure<T>::~Structure() {}

template<typename T> std::set<T*> Structure<T>::selected_items() const
{
  const auto is_selected = [](const auto* t) { return t->is_selected(); };
  return ::filter_if(items(), is_selected);
}

template<typename T> void Structure<T>::invalidate_recursive()
{
  invalidate();
  m_scene.invalidate();
}

template<typename T> size_t Structure<T>::insert_position(const T* predecessor) const
{
  if (predecessor == nullptr) {
    return 0;
  } else {
    return position(*predecessor) + 1;
  }
}

template class Structure<Object>;
template class Structure<Style>;

}  // namespace omm
