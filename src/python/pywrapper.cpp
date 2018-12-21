#include "aspects/propertyowner.h"

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "python/pywrapper.h"
#include "python/objectwrapper.h"
#include "python/stylewrapper.h"
#include "python/tagwrapper.h"
#include "scene/scene.h"
#include "tags/scripttag.h"

namespace py = pybind11;

namespace omm
{

PyWrapper::PyWrapper(void* wrapped)
  : m_wrapped(wrapped)
{
}

py::object wrap(Object* object)
{
  return ObjectWrapper::make(object);
}

py::object wrap(Tag* tag)
{
  return TagWrapper::make(tag);
}

py::object wrap(Style* style)
{
  return py::cast(StyleWrapper(style));
}


}  // namespace omm


