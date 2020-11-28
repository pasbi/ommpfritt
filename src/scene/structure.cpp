#include "scene/structure.h"
#include "common.h"
#include "objects/object.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "tags/tag.h"

namespace omm
{
template<typename T> T* Structure<T>::predecessor(T& sibling) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
  return const_cast<T*>(predecessor(const_cast<const T&>(sibling)));
}

template class Structure<Object>;
template class Structure<Style>;
template class Structure<Tag>;

}  // namespace omm
