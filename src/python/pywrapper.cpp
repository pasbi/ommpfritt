#include "aspects/propertyowner.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "python/pywrapper.h"
#include "python/objectwrapper.h"
#include "python/stylewrapper.h"
#include "python/tagwrapper.h"
#include "scene/scene.h"
#include "renderers/style.h"

namespace py = pybind11;

namespace omm
{

py::object wrap(Object* object)
{
  auto py_object = ObjectWrapper::make(object);
  // PropertyOwnerWrapper::add_property_shortcuts(py_object, *object);
  return py_object;
}

py::object wrap(Tag* tag)
{
  return TagWrapper::make(tag);
}

py::object wrap(Style* style)
{
  return py::cast(StyleWrapper(style));
}

py::object wrap(AbstractPropertyOwner* owner)
{
  if (owner == nullptr) {
    return py::none();
  } else {
    switch (owner->kind()) {
    case AbstractPropertyOwner::Kind::Tag:
      return wrap(static_cast<Tag*>(owner));
    case AbstractPropertyOwner::Kind::Style:
      return wrap(static_cast<Style*>(owner));
    case AbstractPropertyOwner::Kind::Object:
      return wrap(static_cast<Object*>(owner));
    default:
      return py::none();
    }
  }
}


}  // namespace omm


