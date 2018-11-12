#include "objects/objecttransformation.h"
#include "external/json.hpp"

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

ObjectTransformation ObjectTransformation::translated(const Vector2D& translation_vector) const
{
  return *this * translation(translation_vector);
}

ObjectTransformation ObjectTransformation::rotated(double angle) const
{
  return *this * rotation(angle);
}

ObjectTransformation ObjectTransformation::sheared(double shear) const
{
  return *this * shearing(shear);
}

ObjectTransformation ObjectTransformation::scaled(const Vector2D& scale_vector) const
{
  return *this * scalation(scale_vector);
}

ObjectTransformation ObjectTransformation::translation(const Vector2D& translation_vector)
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

ObjectTransformation ObjectTransformation::scalation(const Vector2D& scale_vector)
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
  return arma::all(arma::all(lhs.m_matrix == rhs.m_matrix));
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

ObjectTransformation operator*(const ObjectTransformation& a, const ObjectTransformation& b)
{
  return ObjectTransformation(a.m_matrix * b.m_matrix);
}

}  // namespace omm
