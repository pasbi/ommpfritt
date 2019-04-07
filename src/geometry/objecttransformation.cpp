#include <cassert>
#include <cmath>
#include <ostream>

#include "geometry/objecttransformation.h"

namespace omm
{

ObjectTransformation::ObjectTransformation()
  : m_translation(0, 0), m_scaling(1, 1), m_shearing(0), m_rotation(0) { }

ObjectTransformation::ObjectTransformation( const Vec2f& translation, const Vec2f& scale,
                                            const double rotation, const double shear )
  : m_translation(translation), m_scaling(scale), m_shearing(shear), m_rotation(rotation) { }

ObjectTransformation::ObjectTransformation(const Matrix& mat) { set_mat(mat); }

void ObjectTransformation::set_translation(const Vec2f& translation_vector)
{
  m_translation = translation_vector;
}

void ObjectTransformation::set_rotation(const double& angle)
{
  m_rotation = angle;
}

void ObjectTransformation::set_shearing(const double& shear) { m_shearing = shear; }

void ObjectTransformation::set_scaling(const Vec2f& scale_vector)
{
  m_scaling = scale_vector;
}

void ObjectTransformation::translate(const Vec2f& translation_vector)
{
  m_translation += translation_vector;
}

void ObjectTransformation::rotate(const double& angle) { m_rotation += angle; }
void ObjectTransformation::shear(const double& shear) { m_shearing += shear; }

void ObjectTransformation::scale(const Vec2f& scale_vector)
{
  m_scaling.x *= scale_vector.x;
  m_scaling.y *= scale_vector.y;
}

ObjectTransformation ObjectTransformation::translated(const Vec2f& translation_vector) const
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

ObjectTransformation ObjectTransformation::scaled(const Vec2f& scale_vector) const
{
  auto scaled = *this;
  scaled.scale(scale_vector);
  return scaled;
}

Matrix ObjectTransformation::to_mat() const
{
  const Matrix translation({ { 1, 0, m_translation.x },
                             { 0, 1, m_translation.y },
                             { 0, 0, 1                } });
  const Matrix rotation({ { std::cos(m_rotation), -std::sin(m_rotation), 0 },
                          { std::sin(m_rotation),  std::cos(m_rotation), 0 },
                          { 0,                0,               1 } });
  const Matrix scaling({ { m_scaling.x, 0,              0 },
                         { 0,              m_scaling.y, 0 },
                         { 0,              0,           1 } });
  const Matrix shearing({ { 1,          0, 0 },
                          { m_shearing, 1, 0 },
                          { 0,          0, 1 } });

  return translation * rotation * scaling * shearing;
}

void ObjectTransformation::set_mat(const Matrix& mat)
{
  const double a = mat.m[0][0];
  const double b = mat.m[0][1];
  const double c = mat.m[1][0];
  const double d = mat.m[1][1];

  m_translation = { mat.m[0][2], mat.m[1][2] };

  // TODO NaN can occur if scaling is 0.
  assert(std::abs(mat.m[2][0] - 0) < 0.0001 || std::isnan(mat.m[2][0]));
  assert(std::abs(mat.m[2][1] - 0) < 0.0001 || std::isnan(mat.m[2][1]));
  assert(std::abs(mat.m[2][2] - 1) < 0.0001 || std::isnan(mat.m[2][2]));

  // https://math.stackexchange.com/a/78165/355947
  // translation * scaling * shearing * rotation
  // const double det = a*e - b*d;
  // const double p = sqrt(pow(a, 2.0) + pow(b, 2.0));
  // const double r = det / p;
  // const double q = (a*d + b*e) / det;
  // m_scaling = { p, r };
  // m_shearing = q;
  // m_rotation = atan2(b, a);

  // translation * rotation * scaling * shearing
  const auto n = sqrt(pow(b, 2.0) + pow(d, 2.0));
  m_rotation = -atan2(b, d);
  m_scaling = Vec2f((a*d - b*c) / n, n);
  m_shearing = (a*b + c*d) / pow(n, 2.0);
}

Vec2f ObjectTransformation::translation() const { return m_translation; }
double ObjectTransformation::rotation() const { return m_rotation; }
Vec2f ObjectTransformation::scaling() const { return m_scaling; }
double ObjectTransformation::shearing() const { return m_shearing; }
Vec2f ObjectTransformation::null() const { return apply_to_position(Vec2f::o()); }

std::ostream& operator<<(std::ostream& ostream, const ObjectTransformation& t)
{
  ostream << "[ t(" << t.translation().x << ", " << t.translation().y;
  ostream << ") s(" << t.scaling().x << ", " << t.scaling().y;
  ostream << ") sh(" << t.shearing();
  ostream << ") r(" << t.rotation() * 180 / M_PI << ") ]";
  return ostream;
}

bool operator==(const ObjectTransformation& lhs, const ObjectTransformation& rhs)
{
  return lhs.translation() == rhs.translation()
      && lhs.scaling() == rhs.scaling()
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
    lhs.translation().x - rhs.translation().x,
    lhs.translation().y - rhs.translation().y,
    lhs.scaling().x - rhs.scaling().x,
    lhs.scaling().y - rhs.scaling().y,
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

Vec2f ObjectTransformation::apply_to_position(const Vec2f& position) const
{
  return to_mat().apply_to_position(position);
}

Vec2f ObjectTransformation::apply_to_direction(const Vec2f& direction) const
{
  return to_mat().apply_to_direction(direction);
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

PolarCoordinates ObjectTransformation::apply_to_position(const PolarCoordinates& point) const
{
  // TODO isn't there something smarter?
  return PolarCoordinates(apply_to_position(point.to_cartesian()));
}

PolarCoordinates ObjectTransformation::apply_to_direction(const PolarCoordinates& point) const
{
  // TODO isn't there something smarter?
  return PolarCoordinates(apply_to_direction(point.to_cartesian()));
}


ObjectTransformation ObjectTransformation::inverted() const
{
  // TODO caching the inverse might gain some speed
  return ObjectTransformation(to_mat().inverted());
}

ObjectTransformation ObjectTransformation::normalized() const
{
  ObjectTransformation normalized = *this;
  if (normalized.scaling().x < 0) {
    normalized.set_scaling(-normalized.scaling());
    normalized.set_rotation(normalized.rotation() + M_PI);
  }

  while (normalized.rotation() > M_PI) { normalized.rotate(-2*M_PI); }
  while (normalized.rotation() < -M_PI) { normalized.rotate(2*M_PI); }
  return normalized;
}

bool ObjectTransformation::contains_nan() const
{
  return to_mat().has_nan();
}


ObjectTransformation ObjectTransformation::find_transformation( const ObjectTransformation& from,
                                                                const ObjectTransformation& to )
{
  return to.apply(from.inverted());
}

ObjectTransformation ObjectTransformation::transformed(const ObjectTransformation& other) const
{
  const auto o = other.to_mat();
  return ObjectTransformation(o.inverted() * to_mat() * o);
}


}  // namespace omm
