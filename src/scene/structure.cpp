#include "scene/structure.h"
#include "common.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "tags/tag.h"
#include "scene/scene.h"

namespace omm
{

template<typename T> Structure<T>::Structure(Scene* scene) : m_scene(scene) {}

template<typename T> void Structure<T>::invalidate_recursive()
{
  invalidate();
  if (m_scene != nullptr) {
    m_scene->invalidate();
  }
}

template<typename T> size_t Structure<T>::insert_position(const T* predecessor) const
{
  if (predecessor == nullptr) {
    return 0;
  } else {
    return position(*predecessor) + 1;
  }
}

template<typename T> void Structure<T>::set_scene(Scene* scene)
{
  m_scene = scene;
}

template class Structure<Object>;
template class Structure<Style>;
template class Structure<Tag>;

}  // namespace omm
