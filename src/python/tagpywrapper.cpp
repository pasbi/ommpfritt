#include "python/tagpywrapper.h"
#include "python/objectpywrapper.h"
#include "tags/tag.h"

namespace omm
{

py::object TagPyWrapper::get_owner() const
{
  return py::cast(ObjectPyWrapper(*wrapped->owner()));
}

}  // namespace omm
