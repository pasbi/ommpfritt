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

py::object ObjectWrapper::rotation() const
{
  return py::cast(transformation().rotation());
}

py::object ObjectWrapper::scaling() const
{
  return py::cast(::to_stdvec(transformation().scaling()));
}

py::object ObjectWrapper::translation() const
{
  return py::cast(::to_stdvec(transformation().translation()));
}

py::object ObjectWrapper::set_rotation(const py::object& rotation) const
{
  auto t = transformation();
  t.set_rotation(rotation.cast<double>());
  set_transformation(t);
  return py::cast(true);
}

py::object ObjectWrapper::set_scaling(const py::object& scaling) const
{
  auto t = transformation();
  try {
    t.set_scaling(::to_vec2(scaling.cast<std::vector<double>>()));
  } catch (const std::length_error&) {
    return py::cast(false);
  }
  set_transformation(t);
  return py::cast(true);
}

py::object ObjectWrapper::set_translation(const py::object& translation) const
{
  auto t = transformation();
  try {
    t.set_translation(::to_vec2(translation.cast<std::vector<double>>()));
  } catch (const std::length_error&) {
    return py::cast(false);
  }
  set_transformation(t);
  return py::cast(true);
}

ObjectTransformation ObjectWrapper::transformation() const
{
  auto& property = wrapped.property(Object::TRANSFORMATION_PROPERTY_KEY);
  return property.value<ObjectTransformation>();
}

void ObjectWrapper::set_transformation(const ObjectTransformation& t) const
{
  wrapped.property(Object::TRANSFORMATION_PROPERTY_KEY).set(t);
}

void ObjectWrapper::define_python_interface(py::object& module)
{
  py::class_<ObjectWrapper, AbstractPropertyOwnerWrapper<Object>>(module, wrapped_type::TYPE)
      .def("children", &ObjectWrapper::children)
      .def("parent", &ObjectWrapper::parent)
      .def("tags", &ObjectWrapper::tags)
      .def("rotation", &ObjectWrapper::rotation)
      .def("scaling", &ObjectWrapper::scaling)
      .def("translation", &ObjectWrapper::translation)
      .def("set_rotation", &ObjectWrapper::set_rotation)
      .def("set_translation", &ObjectWrapper::set_translation)
      .def("set_scaling", &ObjectWrapper::set_scaling);
}

}  // namespace omm
