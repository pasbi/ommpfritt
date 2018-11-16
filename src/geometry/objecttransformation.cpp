#include <assert.h>

#include "geometry/objecttransformation.h"

namespace
{

arma::vec2 apply_to_non_homogenous( const omm::ObjectTransformation::mat_type& matrix,
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
ObjectTransformation::ObjectTransformation(const arma::mat::fixed<N_ROWS, N_COLS>& matrix)
  : m_matrix(matrix)
{
}

ObjectTransformation::ObjectTransformation()
{
  m_matrix.eye();
}

ObjectTransformation::ObjectTransformation(const Parameters& parameters)
  : ObjectTransformation()
{
  m_matrix = translation(parameters.translation_vector).m_matrix
           * scalation(parameters.scale_vector).m_matrix
           * shearing(parameters.shear).m_matrix
           * rotation(parameters.rotation).m_matrix;
}

ObjectTransformation ObjectTransformation::translated(const arma::vec2& translation_vector) const
{
  return apply(translation(translation_vector));
}

ObjectTransformation ObjectTransformation::rotated(double angle) const
{
  return apply(rotation(angle));
}

ObjectTransformation ObjectTransformation::sheared(double shear) const
{
  return apply(shearing(shear));
}

ObjectTransformation ObjectTransformation::scaled(const arma::vec2& scale_vector) const
{
  return apply(scalation(scale_vector));
}

ObjectTransformation ObjectTransformation::translation(const arma::vec2& translation_vector)
{
  ObjectTransformation t = identity();
  t.m_matrix.at(0, 2) = translation_vector(0);
  t.m_matrix.at(1, 2) = translation_vector(1);
  return t;
}

ObjectTransformation ObjectTransformation::rotation(double angle)
{
  ObjectTransformation t = identity();
  t.m_matrix(0, 0) = cos(angle);
  t.m_matrix(1, 1) = t.m_matrix(0, 0);
  t.m_matrix(0, 1) = sin(angle);
  t.m_matrix(1, 0) = -t.m_matrix(0, 1);
  return t;
}

ObjectTransformation ObjectTransformation::scalation(const arma::vec2& scale_vector)
{
  ObjectTransformation t = identity();
  t.m_matrix(0, 0) = scale_vector(0);
  t.m_matrix(1, 1) = scale_vector(1);
  return t;
}

ObjectTransformation ObjectTransformation::shearing(double shear)
{
  ObjectTransformation t = identity();
  t.m_matrix(1, 0) = shear;
  return t;
}

ObjectTransformation ObjectTransformation::identity()
{
  return ObjectTransformation();
}

ObjectTransformation::Parameters ObjectTransformation::parameters() const
{
  const double a = m_matrix(0, 0);
  const double b = m_matrix(0, 1);
  const double c = m_matrix(0, 2);
  const double d = m_matrix(1, 0);
  const double e = m_matrix(1, 1);
  const double f = m_matrix(1, 2);
  const double g = m_matrix(2, 0);
  const double h = m_matrix(2, 1);
  const double i = m_matrix(2, 2);
  assert(g == 0);
  assert(h == 0);
  assert(i == 1);

  // https://math.stackexchange.com/a/78165/355947
  const double det = a*e - b*d;
  const double p = sqrt(pow(a, 2.0) + pow(b, 2.0));
  const double r = det / p;
  const double q = (a*d + b*e) / det;
  const double phi = atan2(b, a);

  Parameters parameters;
  parameters.translation_vector = { c, f };
  parameters.rotation = phi;
  parameters.scale_vector = { p, r };
  parameters.shear = q;
  return parameters;
}

arma::vec2 ObjectTransformation::translation() const
{
  return arma::vec2{ m_matrix(0, 2), m_matrix(1, 2) };
}

ObjectTransformation ObjectTransformation::inverted() const
{
  return ObjectTransformation(m_matrix.i());
}

double ObjectTransformation::element(int row, int column) const
{
  return m_matrix(row, column);
}

void ObjectTransformation::set_element(int row, int column, double value)
{
  m_matrix(row, column) = value;
}

std::ostream& operator<<(std::ostream& ostream, const ObjectTransformation& t)
{
  ostream << "[[" << t.element(0, 0) << ", " << t.element(0, 1) << ", " << t.element(0, 2) << "],";
  ostream << " [" << t.element(1, 0) << ", " << t.element(1, 1) << ", " << t.element(1, 2) << "],";
  ostream << " [" << t.element(2, 0) << ", " << t.element(2, 1) << ", " << t.element(2, 2) << "]]";
  return ostream;
}

bool operator==(const ObjectTransformation& lhs, const ObjectTransformation& rhs)
{
  return arma::all(arma::all(lhs.matrix() == rhs.matrix()));
}

bool operator!=(const ObjectTransformation& lhs, const ObjectTransformation& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const ObjectTransformation& lhs, const ObjectTransformation& rhs)
{
  for (int i = 0; i < ObjectTransformation::N_ROWS; ++i) {
    for (int j = 0; j < ObjectTransformation::N_COLS; ++j) {
      const auto a = lhs.element(i, j);
      const auto b = rhs.element(i, j);
      if (a > b) {
        return false;
      } else if (a < b) {
        return true;
      }
    }
  }
  return false; // matrices are equal
}

arma::vec2 ObjectTransformation::apply_to_position(const arma::vec2& position) const
{
  return apply_to_non_homogenous(m_matrix, position, 1.0);
}

arma::vec2 ObjectTransformation::apply_to_direction(const arma::vec2& direction) const
{
  return apply_to_non_homogenous(m_matrix, direction, 0.0);
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
  return ObjectTransformation(m_matrix * t.m_matrix);
}

Point ObjectTransformation::apply(const Point& point) const
{
  Point p(apply_to_position(point.position));
  p.left_tangent = apply_to_direction(point.left_tangent);
  p.right_tangent = apply_to_direction(point.right_tangent);
  return p;
}

const ObjectTransformation::mat_type& ObjectTransformation::matrix() const
{
  return m_matrix;
}

}  // namespace omm
