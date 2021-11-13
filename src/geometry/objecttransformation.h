#pragma once

#include "external/json_fwd.hpp"
#include "geometry/boundingbox.h"
#include "geometry/matrix.h"
#include "geometry/vec2.h"
#include <2geom/pathvector.h>
#include <QTransform>
#include <Qt>

namespace omm
{

class Point;
struct PolarCoordinates;

class ObjectTransformation
{
public:
  explicit ObjectTransformation();
  explicit ObjectTransformation(const Matrix& mat);
  explicit ObjectTransformation(const Vec2f& translation,
                                const Vec2f& scale,
                                double rotation,
                                double shear);

  void set_translation(const Vec2f& translation_vector);
  void translate(const Vec2f& translation_vector);
  [[nodiscard]] ObjectTransformation translated(const Vec2f& translation_vector) const;
  [[nodiscard]] Vec2f translation() const;

  void set_rotation(const double& angle);
  void rotate(const double& angle);
  [[nodiscard]] ObjectTransformation rotated(const double& angle) const;
  [[nodiscard]] double rotation() const;

  void set_scaling(const Vec2f& scale_vector);
  void scale(const Vec2f& scale_vector);
  [[nodiscard]] ObjectTransformation scaled(const Vec2f& scale_vector) const;
  [[nodiscard]] Vec2f scaling() const;

  void set_shearing(const double& shear);
  void shear(const double& shear);
  [[nodiscard]] ObjectTransformation sheared(const double& shear) const;
  [[nodiscard]] double shearing() const;

  [[nodiscard]] ObjectTransformation inverted() const;
  [[nodiscard]] Vec2f null() const;

  [[nodiscard]] ObjectTransformation transformed(const ObjectTransformation& other) const;

  [[nodiscard]] Matrix to_mat() const;
  void set_mat(const Matrix& mat);

  [[nodiscard]] Vec2f apply_to_position(const Vec2f& position) const;
  [[nodiscard]] Vec2f apply_to_direction(const Vec2f& direction) const;
  [[nodiscard]] PolarCoordinates apply_to_position(const PolarCoordinates& point) const;
  [[nodiscard]] PolarCoordinates apply_to_direction(const PolarCoordinates& point) const;
  [[nodiscard]] BoundingBox apply(const BoundingBox& bb) const;
  [[nodiscard]] ObjectTransformation apply(const ObjectTransformation& t) const;
  [[nodiscard]] Point apply(const Point& point) const;
  [[nodiscard]] ObjectTransformation normalized() const;
  [[nodiscard]] bool contains_nan() const;
  [[nodiscard]] bool is_identity() const;

  static constexpr auto TYPE = "ObjectTransformation";

  [[nodiscard]] QTransform to_qtransform() const;

  [[nodiscard]] bool has_nan() const;

  [[nodiscard]] Geom::PathVector apply(const Geom::PathVector& pv) const;
  [[nodiscard]] Geom::Path apply(const Geom::Path& path) const;
  [[nodiscard]] Geom::Point apply(const Geom::Point& point) const;
  [[nodiscard]] std::unique_ptr<Geom::Curve> apply(const Geom::Curve& curve) const;
  operator Geom::Affine() const;

  QString to_string() const;

private:
  Vec2f m_translation = {0.0, 0.0};
  Vec2f m_scaling = {1.0, 1.0};
  double m_shearing = 0.0;
  double m_rotation = 0.0;

  static Geom::PathVector transform(const Geom::PathVector& pv, const Geom::Affine& affine);
  static Geom::Path transform(const Geom::Path& path, const Geom::Affine& affine);
  static std::unique_ptr<Geom::Curve> transform(const Geom::Curve& curve,
                                                const Geom::Affine& affine);
};

bool operator<(const ObjectTransformation& lhs, const ObjectTransformation& rhs);
bool operator==(const ObjectTransformation& lhs, const ObjectTransformation& rhs);
bool operator!=(const ObjectTransformation& lhs, const ObjectTransformation& rhs);

}  // namespace omm
