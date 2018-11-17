#pragma once

#include <armadillo>
#include "external/json_fwd.hpp"
#include "geometry/point.h"
#include "geometry/boundingbox.h"

namespace omm
{

class ObjectTransformation
{
public:
  static constexpr auto N_ROWS = 3;
  static constexpr auto N_COLS = 3;
  using Mat = arma::mat::fixed<N_ROWS, N_COLS>;

  explicit ObjectTransformation();
  explicit ObjectTransformation(const Mat& mat);

  void set_translation(const arma::vec2& translation_vector);
  void translate(const arma::vec2& translation_vector);
  ObjectTransformation translated(const arma::vec2& translation_vector) const;
  arma::vec2 translation() const;

  void set_rotation(const double& angle);
  void rotate(const double& angle);
  ObjectTransformation rotated(const double& angle) const;
  double rotation() const;

  void set_scalation(const arma::vec2& scale_vector);
  void scale(const arma::vec2& scale_vector);
  ObjectTransformation scaled(const arma::vec2& scale_vector) const;
  arma::vec2 scalation() const;

  void set_shearing(const double& shear);
  void shear(const double& shear);
  ObjectTransformation sheared(const double& shear) const;
  double shearing() const;

  ObjectTransformation inverted() const;

  Mat to_mat() const;
  void set_mat(const Mat& mat);

  arma::vec2 apply_to_position(const arma::vec2& position) const;
  arma::vec2 apply_to_direction(const arma::vec2& direction) const;
  BoundingBox apply(const BoundingBox& bb) const;
  ObjectTransformation apply(const ObjectTransformation& t) const;
  Point apply(const Point& point) const;
  ObjectTransformation normalized() const;

private:
  arma::vec2 m_translation;
  arma::vec2 m_scalation;
  double m_shearing;
  double m_rotation;
};

std::ostream& operator<<(std::ostream& ostream, const ObjectTransformation& t);
bool operator<(const ObjectTransformation& a, const ObjectTransformation& b);
bool operator==(const ObjectTransformation& a, const ObjectTransformation& b);
bool operator!=(const ObjectTransformation& a, const ObjectTransformation& b);

}  // namespace omm
