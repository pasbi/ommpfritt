#pragma once

#include "python/propertyownerwrapper.h"
#include "python/wrapperfactory.h"

namespace omm
{

class ObjectWrapper
  : public AbstractPropertyOwnerWrapper<Object>
  , public WrapperFactory<Object, ObjectWrapper>
{
public:
  using AbstractPropertyOwnerWrapper<Object>::AbstractPropertyOwnerWrapper;
  py::object children() const;
  py::object parent() const;
  py::object tags() const;
  py::object rotation() const;
  py::object scaling() const;
  py::object translation() const;
  py::object set_rotation(const py::object& rotation) const;
  py::object set_translation(const py::object& translation) const;
  py::object set_scaling(const py::object& scaling) const;

  static void define_python_interface(py::object& module);
  using wrapped_type = Object;

private:
  ObjectTransformation transformation() const;
  void set_transformation(const ObjectTransformation& t) const;
};

}  // namespace omm
