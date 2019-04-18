#include "geometry/matrix.h"
#include <cmath>

namespace omm
{

Matrix::Matrix(const std::array<std::array<double, 3>, 3>& ll) : m(ll) {}
Matrix::Matrix(const std::initializer_list<std::initializer_list<double>>& ll)
{
  for (std::size_t i = 0; i < 3; ++i) {
    for (std::size_t j = 0; j < 3; ++j) {
      m[i][j] = ll.begin()[i].begin()[j];
    }
  }
}

Matrix::Matrix(Initialization initialization)
{
  if (initialization == Initialization::Identity) {
    m[0][0] = 1.0;
    m[0][1] = 0.0;
    m[0][2] = 0.0;
    m[1][0] = 0.0;
    m[1][1] = 1.0;
    m[1][2] = 0.0;
    m[2][0] = 0.0;
    m[2][1] = 0.0;
    m[2][2] = 1.0;
  } else if (initialization == Initialization::Zeros) {
    m[0][0] = 0.0;
    m[0][1] = 0.0;
    m[0][2] = 0.0;
    m[1][0] = 0.0;
    m[1][1] = 0.0;
    m[1][2] = 0.0;
    m[2][0] = 0.0;
    m[2][1] = 0.0;
    m[2][2] = 0.0;
  } else {
    assert(initialization == Initialization::None);
  }
}

Matrix Matrix::inverted() const
{
  // https://stackoverflow.com/a/18504573/4248972
  const double det = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
                     m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                     m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
  const double i_det = 1.0 / det;

  Matrix inv(Initialization::None); // inverse of matrix m
  inv.m[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * i_det;
  inv.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * i_det;
  inv.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * i_det;
  inv.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * i_det;
  inv.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * i_det;
  inv.m[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * i_det;
  inv.m[2][0] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * i_det;
  inv.m[2][1] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * i_det;
  inv.m[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * i_det;
  return inv;
}

Matrix Matrix::operator*(const Matrix& other) const
{
  Matrix p(Initialization::Zeros);
  for (std::size_t i = 0; i < 3; ++i) {
    for (std::size_t j = 0; j < 3; ++j) {
      for (std::size_t k = 0; k < 3; ++k) {
        p.m[i][j] += m[i][k] * other.m[k][j];
      }
    }
  }
  return p;
}

Vec2f Matrix::apply_to_position(const Vec2f& p) const
{
  return apply(std::vector{ p.x, p.y, 1.0 });
}

Vec2f Matrix::apply_to_direction(const Vec2f& d) const
{
  return apply(std::vector{ d.x, d.y, 0.0 });
}

Vec2f Matrix::apply(const std::vector<double>& vec) const
{
  std::vector<double> result { 0.0, 0.0, 0.0 };
  for (std::size_t i = 0; i < 3; ++i) {
    for (std::size_t j = 0; j < 3; ++j) {
      result[i] += vec[j] * m[i][j];
    }
  }
  // usually, this is true. but it fails if scale.y becomes zero.
  // assert(std::abs(vec[2] - result[2]) < 0.0001);

  return Vec2f(result[0], result[1]);
}

bool Matrix::has_nan() const
{
  for (std::size_t i = 0; i < 3; ++i) {
    for (std::size_t j = 0; j < 3; ++j) {
      if (std::isnan(m[i][j])) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace omm
