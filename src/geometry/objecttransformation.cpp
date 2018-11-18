#include <assert.h>

#include "geometry/objecttransformation.h"

namespace
{

arma::vec2 apply_to_non_homogenous( const omm::ObjectTransformation::Mat& matrix,
                                    const arma::vec2& vector,
                                    const double third_component )
{
  auto homogenous_coordinates = arma::vec3 { vector[0], vector[1], third_component };
  homogenous_coordinates = matrix * homogenous_coordinates;

  // assert kind of vector did not change (direction maps to direction, position maps to position)
  assert(homogenous_coordinates[2] == third_component);

  return { homogenous_coordinates[0], homogenous_coordinates[1] };
}

}  // namespace

namespace omm
{

 ObjectTransformation::ObjectTransformation()
  : m_translation({ 0, 0 })
  , m_scalation({ 1, 1 })
  , m_shearing(0)
  , m_rotation(0)
{
}

ObjectTransformation::ObjectTransformation(const Mat& mat)
{
  set_mat(mat);
}

void ObjectTransformation::set_translation(const arma::vec2& translation_vector)
{
  m_translation = translation_vector;
}

void ObjectTransformation::set_rotation(const double& angle)
{
  m_rotation = angle;
}

void ObjectTransformation::set_shearing(const double& shear)
{
  m_shearing = shear;
}

void ObjectTransformation::set_scalation(const arma::vec2& scale_vector)
{
  m_scalation = scale_vector;
}

void ObjectTransformation::translate(const arma::vec2& translation_vector)
{
  m_translation += translation_vector;
}

void ObjectTransformation::rotate(const double& angle)
{
  m_rotation += angle;
}

void ObjectTransformation::shear(const double& shear)
{
  m_shearing += shear;
}

void ObjectTransformation::scale(const arma::vec2& scale_vector)
{
  m_scalation(0) *= scale_vector(0);
  m_scalation(1) *= scale_vector(1);
}

ObjectTransformation ObjectTransformation::translated(const arma::vec2& translation_vector) const
{
  auto translated = *this;
  translated.translate(translation_vector);
  return translated;
}

ObjectTransformation ObjectTransformation::rotated(const double& angle) const
{
  auto rotated = *this;
  rotated.rotate(angle);
  return rotated;
}

ObjectTransformation ObjectTransformation::sheared(const double& shear) const
{
  auto sheared = *this;
  sheared.shear(shear);
  return sheared;
}

ObjectTransformation ObjectTransformation::scaled(const arma::vec2& scale_vector) const
{
  auto scaled = *this;
  scaled.scale(scale_vector);
  return scaled;
}

ObjectTransformation::Mat ObjectTransformation::to_mat() const
{
  const Mat translation = { { 1, 0, m_translation(0) },
                            { 0, 1, m_translation(1) },
                            { 0, 0, 1                } };
  const Mat rotation = { { cos(m_rotation), -sin(m_rotation), 0 },
                         { sin(m_rotation),  cos(m_rotation), 0 },
                         { 0,                0,               1 } };
  const Mat scalation = { { m_scalation(0), 0,              0 },
                          { 0,              m_scalation(1), 0 },
                          { 0,              0,              1 } };
  const Mat shearing = { { 1,          0, 0 },
                         { m_shearing, 1, 0 },
                         { 0,          0, 1 } };

  return translation * rotation * scalation * shearing;
}

void ObjectTransformation::set_mat(const Mat& mat)
{
  const double a = mat(0, 0);
  const double b = mat(0, 1);
  const double c = mat(1, 0);
  const double d = mat(1, 1);

  m_translation = { mat(0, 2), mat(1, 2) };

  assert(mat(2, 0) == 0);
  assert(mat(2, 1) == 0);
  assert(mat(2, 2) == 1);

  // https://math.stackexchange.com/a/78165/355947
  // translation * scalation * shearing * rotation
  // const double det = a*e - b*d;
  // const double p = sqrt(pow(a, 2.0) + pow(b, 2.0));
  // const double r = det / p;
  // const double q = (a*d + b*e) / det;
  // m_scalation = { p, r };
  // m_shearing = q;
  // m_rotation = atan2(b, a);

  // translation * rotation * scalation * shearing
  const auto n = sqrt(pow(b, 2.0) + pow(d, 2.0));
  m_rotation = -atan2(b, d);
  m_scalation(1) = n;
  m_scalation(0) = (a*d - b*c) / n;
  m_shearing = (a*b + c*d) / pow(n, 2.0);
}

arma::vec2 ObjectTransformation::translation() const
{
  return m_translation;
}

double ObjectTransformation::rotation() const
{
  return m_rotation;
}

arma::vec2 ObjectTransformation::scalation() const
{
  return m_scalation;
}

double ObjectTransformation::shearing() const
{
  return m_shearing;
}

std::ostream& operator<<(std::ostream& ostream, const ObjectTransformation& t)
{
  ostream << "[ t(" << t.translation()(0) << ", " << t.translation()(1);
  ostream << ") s(" << t.scalation()(0) << ", " << t.scalation()(1);
  ostream << ") sh(" << t.shearing();
  ostream << ") r(" << t.rotation() * 180 / M_PI << ") ]";
  return ostream;
}

bool operator==(const ObjectTransformation& lhs, const ObjectTransformation& rhs)
{
  return arma::all(lhs.translation() == rhs.translation())
      && arma::all(lhs.scalation() == rhs.scalation())
      && lhs.rotation() == rhs.rotation()
      && lhs.shearing() == rhs.shearing();
}

bool operator!=(const ObjectTransformation& lhs, const ObjectTransformation& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const ObjectTransformation& lhs, const ObjectTransformation& rhs)
{
  const std::vector<double> diffs = {
    lhs.translation()(0) - rhs.translation()(0),
    lhs.translation()(1) - rhs.translation()(1),
    lhs.scalation()(0) - rhs.scalation()(0),
    lhs.scalation()(1) - rhs.scalation()(1),
    lhs.rotation() - rhs.rotation(),
    lhs.shearing() - rhs.shearing()
  };

  for (auto d : diffs) {
    if (d < 0) {
      return true;
    } else if (d > 0) {
      return false;
    }
  }
  return false;
}

arma::vec2 ObjectTransformation::apply_to_position(const arma::vec2& position) const
{
  return apply_to_non_homogenous(to_mat(), position, 1.0);
}

arma::vec2 ObjectTransformation::apply_to_direction(const arma::vec2& direction) const
{
  return apply_to_non_homogenous(to_mat(), direction, 0.0);
}

BoundingBox ObjectTransformation::apply(const BoundingBox& bb) const
{
  const auto top_left = apply_to_position(bb.top_left());
  const auto top_right = apply_to_position(bb.top_right());
  const auto bottom_left = apply_to_position(bb.bottom_left());
  const auto bottom_right = apply_to_position(bb.bottom_right());

  return BoundingBox({top_left, top_right, bottom_left, bottom_right});
}

ObjectTransformation ObjectTransformation::apply(const ObjectTransformation& t) const
{
  return ObjectTransformation(to_mat() * t.to_mat());
}

Point ObjectTransformation::apply(const Point& point) const
{
  Point p(apply_to_position(point.position));
  p.left_tangent = apply_to_direction(point.left_tangent);
  p.right_tangent = apply_to_direction(point.right_tangent);
  return p;
}

ObjectTransformation ObjectTransformation::inverted() const
{
  return ObjectTransformation(to_mat().i());
}

ObjectTransformation ObjectTransformation::normalized() const
{
  ObjectTransformation normalized = *this;
  if (normalized.scalation()(0) < 0) {
    normalized.set_scalation(-normalized.scalation());
    normalized.set_rotation(normalized.rotation() + M_PI);
  }
  return normalized;
}

}  // namespace omm
