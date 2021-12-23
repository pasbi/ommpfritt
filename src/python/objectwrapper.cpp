#include "python/objectwrapper.h"

namespace omm
{
py::object ObjectWrapper::children() const
{
  return wrap(wrapped.tree_children());
}

py::object ObjectWrapper::parent() const
{
  if (this->wrapped.is_root()) {
    return py::none();
  } else {
    return wrap(this->wrapped.tree_parent());
  }
}

py::object ObjectWrapper::tags() const
{
  return wrap(wrapped.tags.ordered_items());
}

py::object ObjectWrapper::update()
{
  this->wrapped.update();
  return py::none();
}

void ObjectWrapper::define_python_interface(py::object& module)
{
  py::class_<ObjectWrapper, AbstractPropertyOwnerWrapper<Object>>(module, wrapped_type::TYPE)
      .def("children", &ObjectWrapper::children)
      .def("parent", &ObjectWrapper::parent)
      .def("tags", &ObjectWrapper::tags)
      .def("update", &ObjectWrapper::update);
}

}  // namespace omm
