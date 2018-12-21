#include "aspects/propertyowner.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "python/pywrapper.h"
#include "scene/scene.h"
#include "tags/scripttag.h"

namespace py = pybind11;

namespace omm
{

py::object PropertyOwnerWrapper::property(const std::string& key) const
{
  return py::cast(wrapped<AbstractPropertyOwner>().property(key).variant_value());
}

void PropertyOwnerWrapper::set(const std::string& key, const py::object& value) const
{
  wrapped<AbstractPropertyOwner>().property(key).set(value.cast<Property::variant_type>());
}

py::object TagWrapper::owner() const
{
  return wrap(*wrapped<Tag>().owner());
}

py::object ObjectWrapper::children() const
{
  return wrap(wrapped<Object>().children());
}

py::object ObjectWrapper::parent() const
{
  Object& wrapped = this->wrapped<Object>();
  if (wrapped.is_root()) {
    return py::none();
  } else {
    return wrap(wrapped.parent());
  }
}

py::object ObjectWrapper::tags() const
{
  return wrap(wrapped<Object>().tags.ordered_items());
}

py::object wrap(Object& object)
{
  const auto type = object.type();
  if (false) {
    return py::none(); // TODO
  } else {
    return py::cast(ObjectWrapper(&object));
  }
}

py::object wrap(Scene& scene)
{
  return py::cast(SceneWrapper(&scene));
}

py::object wrap(Style& style)
{
  return py::cast(StyleWrapper(&style));
}

py::object wrap(Tag& tag)
{
  const auto type = tag.type();
  if (type == ScriptTag::TYPE) {
    return py::cast(ScriptTagWrapper(&tag));
  } else {
    return py::cast(TagWrapper(&tag));
  }
}


}  // namespace omm


