#include "python/objectpywrapper.h"
#include <pybind11/stl.h>
#include "objects/object.h"

namespace omm
{

py::object ObjectPyWrapper::get_parent()
{
  if (wrapped->is_root()) {
    return py::none();
  } else {
    return py::cast(ObjectPyWrapper(wrapped->parent()));
  }
}

py::object ObjectPyWrapper::get_children()
{
  return py::cast(::transform<ObjectPyWrapper>(wrapped->children(), [](Object* child) {
    return ObjectPyWrapper(*child);
  }));
}

}  // namespace omm
