#include "python/objectwrapper.h"

namespace omm
{

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
    return wrap(&wrapped.parent());
  }
}

py::object ObjectWrapper::tags() const
{
  return wrap(wrapped<Object>().tags.ordered_items());
}

void ObjectWrapper::define_python_interface(py::object& module)
{
  py::class_<ObjectWrapper, PropertyOwnerWrapper>(module, wrapped_type::TYPE)
      .def("children", &ObjectWrapper::children)
      .def("parent", &ObjectWrapper::parent)
      .def("tags", &ObjectWrapper::tags);
}

}  // namespace omm
