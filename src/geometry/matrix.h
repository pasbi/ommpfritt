#pragma once

#include "geometry/vec2.h"

namespace omm
{

class Matrix
{
public:
  enum class Initialization { Zeros, Identity, None };
  explicit Matrix(Initialization initialization = Initialization::Identity);
  explicit Matrix(const std::array<std::array<double, 3>, 3>& ll);
  explicit Matrix(const std::initializer_list<std::initializer_list<double>>& ll);
  std::array<std::array<double, 3>, 3> m;
  Matrix inverted() const;
  Matrix operator*(const Matrix& other) const;
  Vec2f apply_to_position(const Vec2f& p) const;
  Vec2f apply_to_direction(const Vec2f& d) const;

  static Matrix zeros();
  bool has_nan() const;

private:
  Vec2f apply(const std::vector<double>& vec) const;
};

};
