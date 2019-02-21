#pragma once

#include <armadillo>
#include "external/json_fwd.hpp"
#include "geometry/point.h"
#include "geometry/boundingbox.h"
#include <Qt>

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
  explicit ObjectTransformation( const arma::vec2& translation, const arma::vec2& scale,
                                 const double rotation, const double shear );

  void set_translation(const arma::vec2& translation_vector);
  void translate(const arma::vec2& translation_vector);
  ObjectTransformation translated(const arma::vec2& translation_vector) const;
  arma::vec2 translation() const;

  void set_rotation(const double& angle);
  void rotate(const double& angle);
  ObjectTransformation rotated(const double& angle) const;
  double rotation() const;

  void set_scaling(const arma::vec2& scale_vector);
  void scale(const arma::vec2& scale_vector);
  ObjectTransformation scaled(const arma::vec2& scale_vector) const;
  arma::vec2 scaling() const;

  void set_shearing(const double& shear);
  void shear(const double& shear);
  ObjectTransformation sheared(const double& shear) const;
  double shearing() const;

  ObjectTransformation inverted() const;
  static ObjectTransformation find_transformation( const ObjectTransformation& from,
                                                   const ObjectTransformation& to );

  ObjectTransformation transformed(const ObjectTransformation& other) const;

  Mat to_mat() const;
  void set_mat(const Mat& mat);

  arma::vec2 apply_to_position(const arma::vec2& position) const;
  arma::vec2 apply_to_direction(const arma::vec2& direction) const;
  PolarCoordinates apply_to_position(const PolarCoordinates& point) const;
  PolarCoordinates apply_to_direction(const PolarCoordinates& point) const;
  BoundingBox apply(const BoundingBox& bb) const;
  ObjectTransformation apply(const ObjectTransformation& t) const;
  Point apply(const Point& point) const;
  OrientedPoint apply(const OrientedPoint& point);
  double apply_to_angle(double angle) const;
  ObjectTransformation normalized() const;
  bool contains_nan() const;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("ObjectTransformation", "ObjectTransformation");

private:
  arma::vec2 m_translation;
  arma::vec2 m_scaling;
  double m_shearing;
  double m_rotation;
};

std::ostream& operator<<(std::ostream& ostream, const ObjectTransformation& t);
bool operator<(const ObjectTransformation& a, const ObjectTransformation& b);
bool operator==(const ObjectTransformation& a, const ObjectTransformation& b);
bool operator!=(const ObjectTransformation& a, const ObjectTransformation& b);

}  // namespace omm
