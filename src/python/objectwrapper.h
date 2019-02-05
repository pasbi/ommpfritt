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
  // py::object rotation() const;
  // py::object scale() const;
  // py::object translation() const;
  // py::object shear() const;
  // py::object set_rotation(const py::object& rotation) const;
  // py::object set_translation(const py::object& translation) const;
  // py::object set_scale(const py::object& scale) const;
  // py::object set_shear(const py::object& shear) const;

  static void define_python_interface(py::object& module);
  using wrapped_type = Object;
};

}  // namespace omm
