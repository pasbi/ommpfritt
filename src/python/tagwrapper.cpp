#include "python/tagwrapper.h"

namespace omm
{
py::object TagWrapper::owner() const
{
  return wrap(*wrapped.owner);
}

void TagWrapper::define_python_interface(py::object& module)
{
  py::class_<TagWrapper, AbstractPropertyOwnerWrapper<Tag>>(module, wrapped_type::TYPE)
      .def("owner", &TagWrapper::owner);
}

}  // namespace omm
