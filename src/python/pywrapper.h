#pragma once

#include "external/pybind11/embed.h"
#include "external/pybind11/stl.h"

#include "aspects/propertyowner.h"
#include "scene/scene.h"
#include "splinetype.h"
#include "tags/scripttag.h"

namespace py = pybind11;

namespace omm
{
py::object variant_to_python(const variant_type& variant);
variant_type python_to_variant(const py::object& object, const QString& type);

py::object wrap(Object& object);
py::object wrap(Tag& tag);
py::object wrap(Style& style);
py::object wrap(AbstractPropertyOwner* owner);
py::object wrap(SplineType& spline);

template<typename Ts> py::object wrap(const Ts& items)
{
  using item_type = typename Ts::value_type;
  return py::cast(
      ::transform<py::object, std::list>(items, [](item_type item) { return wrap(*item); }));
}

class AbstractPyWrapper
{
public:
  virtual ~AbstractPyWrapper() = default;
};

template<typename WrappedT> class PyWrapper : AbstractPyWrapper
{
public:
  explicit PyWrapper(WrappedT& wrapped) : wrapped(wrapped)
  {
  }
  using wrapped_type = WrappedT;
  WrappedT& wrapped;
};

}  // namespace omm
