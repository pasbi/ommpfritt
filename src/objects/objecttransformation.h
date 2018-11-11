#pragma once

#include <armadillo>
#include "external/json_fwd.hpp"

namespace omm
{

class ObjectTransformation
{
public:
  using Vector2D = arma::vec::fixed<2>;
  static constexpr auto N_ROWS = 3;
  static constexpr auto N_COLS = 3;
  struct Parameters
  {
    Vector2D translation_vector;
    double rotation;
    Vector2D scale_vector;
    double shear;
  };

  explicit ObjectTransformation();
  explicit ObjectTransformation(const Parameters& parameters);
  ObjectTransformation translated(const Vector2D& translation_vector) const;
  ObjectTransformation rotated(double angle) const;
  ObjectTransformation scaled(const Vector2D& scale_vector) const;
  ObjectTransformation sheared(double shear) const;
  ObjectTransformation inverted() const;

  static ObjectTransformation translation(const Vector2D& translation_vector);
  static ObjectTransformation rotation(double angle);
  static ObjectTransformation scalation(const Vector2D& scale_vector);
  static ObjectTransformation shearing(double shear);
  static ObjectTransformation identity();

  Parameters parameters() const;
  double element(int row, int column) const;
  void set_element(int row, int column, double value);

private:
  explicit ObjectTransformation(const arma::mat::fixed<N_ROWS, N_COLS>& matrix);
  arma::mat::fixed<N_ROWS, N_COLS> m_matrix;

  friend ObjectTransformation operator*(const ObjectTransformation&, const ObjectTransformation&);
  friend bool operator==(const ObjectTransformation&, const ObjectTransformation&);

};

std::ostream& operator<<(std::ostream& ostream, const ObjectTransformation& t);
bool operator<(const ObjectTransformation& a, const ObjectTransformation& b);
bool operator==(const ObjectTransformation& a, const ObjectTransformation& b);
ObjectTransformation operator*(const ObjectTransformation& a, const ObjectTransformation& b);

}  // namespace omm