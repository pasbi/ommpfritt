#include "python/objectwrapper.h"
#include "geometry/util.h"

namespace omm
{

py::object ObjectWrapper::children() const
{
  return wrap(wrapped.children());
}

py::object ObjectWrapper::parent() const
{
  if (this->wrapped.is_root()) {
    return py::none();
  } else {
    return wrap(this->wrapped.parent());
  }
}

py::object ObjectWrapper::tags() const
{
  return wrap(wrapped.tags.ordered_items());
}

// py::object ObjectWrapper::rotation() const
// {
//   return py::cast(transformation().rotation());
// }

// py::object ObjectWrapper::scale() const
// {
//   return py::cast(::to_stdvec(transformation().scaling()));
// }

// py::object ObjectWrapper::translation() const
// {
//   return py::cast(::to_stdvec(transformation().translation()));
// }

// py::object ObjectWrapper::set_rotation(const py::object& rotation) const
// {
//   wrapped.property(Object::ROTATION_PROPERTY_KEY).set(rotation.cast<double>());
//   return py::cast(true);
// }

// py::object ObjectWrapper::set_scale(const py::object& scale) const
// {
//   try {
//     const auto py_scale = ::to_vec2(scale.cast<std::vector<double>>());
//     wrapped.property(Object::SCALE_PROPERTY_KEY).set(py_scale);
//   } catch (const std::length_error&) {
//     return py::cast(false);
//   }
//   return py::cast(true);
// }

// py::object ObjectWrapper::set_shear(const py::object& shear) const
// {
//   wrapped.property(Object::SHEAR_PROPERTY_KEY).set(shear.cast<double>());
//   return py::cast(true);
// }

// py::object ObjectWrapper::set_translation(const py::object& translation) const
// {
//   try {
//     const auto py_translation = ::to_vec2(translation.cast<std::vector<double>>());
//     wrapped.property(Object::SCALE_PROPERTY_KEY).set(py_translation);
//   } catch (const std::length_error&) {
//     return py::cast(false);
//   }
//   return py::cast(true);
// }

void ObjectWrapper::define_python_interface(py::object& module)
{
  py::class_<ObjectWrapper, AbstractPropertyOwnerWrapper<Object>>(module, wrapped_type::TYPE)
      .def("children", &ObjectWrapper::children)
      .def("parent", &ObjectWrapper::parent)
      .def("tags", &ObjectWrapper::tags);
      // .def("rotation", &ObjectWrapper::rotation)
      // .def("scale", &ObjectWrapper::scale)
      // .def("shear", &ObjectWrapper::shear)
      // .def("translation", &ObjectWrapper::translation)
      // .def("set_rotation", &ObjectWrapper::set_rotation)
      // .def("set_translation", &ObjectWrapper::set_translation)
      // .def("set_scale", &ObjectWrapper::scale)
      // .def("set_shear", &ObjectWrapper::set_shear);
}

}  // namespace omm
