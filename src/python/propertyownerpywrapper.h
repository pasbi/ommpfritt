#pragma once

#include <pybind11/embed.h>

namespace py = pybind11;

namespace omm
{

class AbstractPropertyOwner;
class PropertyOwnerPyWrapper
{
public:
  explicit PropertyOwnerPyWrapper(AbstractPropertyOwner& wrapped);
  py::object property(const std::string& key) const;
  void set(const std::string& key, const py::object& value) const;

private:
  AbstractPropertyOwner* m_wrapped;
  friend bool operator<(const PropertyOwnerPyWrapper& a, const PropertyOwnerPyWrapper& b);
};

bool operator<(const PropertyOwnerPyWrapper& a, const PropertyOwnerPyWrapper& b);

}  // namespace omm
