#pragma once

#include "geometry/point.h"
#include "python/pywrapper.h"

namespace omm
{
class PointWrapper : public AbstractPyWrapper
{
public:
  explicit PointWrapper(const Point& point);
  static void define_python_interface(py::object& module);
  [[nodiscard]] py::object left_tangent() const;
  [[nodiscard]] py::object right_tangent() const;
  [[nodiscard]] py::object position() const;
  void set_left_tangent(const py::object& value);
  void set_right_tangent(const py::object& value);
  void set_position(const py::object& value);

private:
  Point m_point;
};

}  // namespace omm
