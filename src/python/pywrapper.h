#pragma once

#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "aspects/propertyowner.h"
#include "scene/scene.h"
#include "tags/scripttag.h"

namespace py = pybind11;

namespace omm
{

py::object variant_to_python(variant_type variant);
variant_type python_to_variant(const py::object& object, const QString& type);

py::object wrap(Object& object);
py::object wrap(Tag& tag);
py::object wrap(Style& style);
py::object wrap(AbstractPropertyOwner* owner);

template<typename Ts>
py::object wrap(const Ts& items)
{
  using item_type = typename Ts::value_type;
  return py::cast(::transform<py::object, std::list>(items, [](item_type item) {
    return wrap(*item);
  }));
}

template<typename WrappedT>
class PyWrapper
{
public:
  explicit PyWrapper(WrappedT& wrapped) : wrapped(wrapped) { }
  virtual ~PyWrapper() = default;
  using wrapped_type = WrappedT;
  WrappedT& wrapped;
};

}  // namespace omm


