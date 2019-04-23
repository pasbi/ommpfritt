#include "scene/structure.h"
#include "common.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "tags/tag.h"
#include "scene/scene.h"

namespace omm
{

template<typename T> size_t Structure<T>::insert_position(const T* predecessor) const
{
  if (predecessor == nullptr) {
    return 0;
  } else {
    return position(*predecessor) + 1;
  }
}

template<typename T> T* Structure<T>::predecessor(T& sibling) const
{
  return const_cast<T*>(predecessor(static_cast<const T&>(sibling)));
}

template class Structure<Object>;
template class Structure<Style>;
template class Structure<Tag>;

}  // namespace omm
