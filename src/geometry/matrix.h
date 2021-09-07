#pragma once

#include "geometry/vec2.h"

#include <QGenericMatrix>
#include <QTransform>

namespace omm
{
class Matrix
{
public:
  enum class Initialization { Zeros, Identity, None };
  explicit Matrix(Initialization initialization = Initialization::Identity);
  explicit Matrix(const std::array<std::array<double, 3>, 3>& ll);
  explicit Matrix(const std::initializer_list<std::initializer_list<double>>& ll);
  std::array<std::array<double, 3>, 3> m{};
  [[nodiscard]] Matrix inverted() const;
  Matrix operator*(const Matrix& other) const;
  [[nodiscard]] Vec2f apply_to_position(const Vec2f& p) const;
  [[nodiscard]] Vec2f apply_to_direction(const Vec2f& d) const;

  static Matrix zeros();
  [[nodiscard]] bool has_nan() const;

  [[nodiscard]] QTransform to_qtransform() const;
  [[nodiscard]] QMatrix3x3 to_qmatrix3x3() const;

private:
  [[nodiscard]] Vec2f apply(const std::vector<double>& vec) const;
};

}  // namespace omm
