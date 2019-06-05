#include "geometry/point.h"
#include <cmath>
#include <ostream>
#include "logging.h"

namespace omm
{

Point::Point(const Vec2f& position,
             const PolarCoordinates& left_tangent, const PolarCoordinates& right_tangent)
  : position(position), left_tangent(left_tangent), right_tangent(right_tangent)
{
}

Point::Point(const Vec2f& position, const double rotation, const double tangent_length)
  : Point( position, PolarCoordinates(rotation, tangent_length),
                     PolarCoordinates(M_PI + rotation, tangent_length) )
{
}

Point::Point(const Vec2f& position) : Point(position, 0.0, 0.0) { }
Point::Point() : Point(Vec2f::o()) { }

Vec2f Point::left_position() const { return position + left_tangent.to_cartesian(); }
Vec2f Point::right_position() const { return position + right_tangent.to_cartesian(); }

void Point::swap(Point& other)
{
  other.position.swap(position);
  other.left_tangent.swap(left_tangent);
  other.right_tangent.swap(right_tangent);
}

Point Point::smoothed(Vec2f& left, const Vec2f& right) const
{
  auto copy = *this;
  const Vec2f d = left - right;
  copy.right_tangent = PolarCoordinates(-d/6.0);
  copy.left_tangent = PolarCoordinates(d/6.0);
  return copy;
}

double Point::rotation() const
{
  return PolarCoordinates(left_tangent).argument;
}

Point Point::nibbed() const
{
  auto copy = *this;
  copy.left_tangent.magnitude = 0;
  copy.right_tangent.magnitude = 0;
  return copy;
}

Point Point::rotated(const double rad) const
{
  auto copy = *this;
  copy.left_tangent.argument += rad;
  copy.right_tangent.argument += rad;
  return copy;
}

Point Point::flattened(const double t) const
{
  Point copy(*this);
  auto center = 0.5*(left_tangent.argument + right_tangent.argument);
  if (center - left_tangent.argument < 0) {
    center += M_PI;
  }

  const auto lerp_angle = [](double rad1, double rad2, const double t) {

    const Vec2f v =   t   * PolarCoordinates(rad2, 1.0).to_cartesian()
                  + (1-t) * PolarCoordinates(rad1, 1.0).to_cartesian();
    return (v/2.0).arg();

  };

  copy.left_tangent.argument = lerp_angle(left_tangent.argument, center - M_PI_2, t);
  copy.right_tangent.argument = lerp_angle(right_tangent.argument, center + M_PI_2, t);

  return copy;
}

std::ostream& operator<<(std::ostream& ostream, const PolarCoordinates& pc)
{
  ostream << "[phi=" << pc.argument << ", r=" << pc.magnitude << "]";
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Point& pc)
{
  static constexpr bool verbose = false;
  if constexpr (verbose) {
    ostream << "Point[[" << pc.position.x << ", " << pc.position.y << "], " << pc.left_tangent
            << ", " << pc.right_tangent << "]";
  } else {
    ostream << "[" << pc.position.x << ", " << pc.position.y << "]";
  }
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Point* pc)
{
  if (pc == nullptr) {
    ostream << "Point[nullptr]";
  }
  ostream << *pc;
  return ostream;
}

bool Point::operator==(const Point& point) const
{
  return position == point.position
    && left_tangent == point.left_tangent
    && right_tangent == point.right_tangent;
}

bool Point::operator!=(const Point& point) const { return !(*this == point); }

PolarCoordinates Point::mirror_tangent( const PolarCoordinates& old_pos,
                                        const PolarCoordinates& old_other_pos,
                                        const PolarCoordinates& new_other_pos )
{
  PolarCoordinates new_pos;
  static constexpr double mag_eps = 0.00001;
  new_pos.argument = old_pos.argument + new_other_pos.argument - old_other_pos.argument;
  if (old_other_pos.magnitude > mag_eps) {
    new_pos.magnitude = old_pos.magnitude * new_other_pos.magnitude / old_other_pos.magnitude;
  } else {
    new_pos.magnitude = new_other_pos.magnitude;
  }
  return new_pos;
}

double Point::get_direction(const Point *left_neighbor, const Point *right_neighbor) const
{
  double left_arg = 0;
  double right_arg = 0;
  bool has_left_direction = true;
  bool has_right_direction = true;
  static constexpr auto eps = 0.001;

  if (right_tangent.magnitude >= eps) {
    left_arg = right_tangent.argument;
  } else if (left_neighbor != nullptr) {
    left_arg = (position - left_neighbor->position).arg();
  } else {
    has_left_direction = false;
  }

  if (left_tangent.magnitude >= eps) {
    right_arg = left_tangent.argument;
  } else if (right_neighbor != nullptr) {
    right_arg = (position - right_neighbor->position).arg();
  } else {
    has_right_direction = false;
  }

  if (has_left_direction && has_right_direction) {
    auto a = 0.5  * (left_arg + right_arg);
    if (a - right_arg < 0) {
      a -= M_PI;
    }
    return a;

  } else if (has_left_direction) {
    return left_arg + M_PI_2;
  } else if (has_right_direction) {
    return right_arg - M_PI_2;
  } else {
    LWARNING << "Directed point must have at least one neighbor or tangent";
    return 0.0;
  }
}

Point Point::offset(double t, const Point *left_neighbor, const Point *right_neighbor) const
{
  const double arg = get_direction(left_neighbor, right_neighbor);
  const auto direction = PolarCoordinates(arg, 1.0).to_cartesian();
  const Vec2f pdirection = direction;  // (direction.y, -direction.x);

  const auto f = [](const double t, const double mag) {
    return mag + std::clamp(t, -mag, 0.0) + 0.5 * (std::max(0.0, t));
  };

  auto left_tanget = this->left_tangent;
  auto right_tangent = this->right_tangent;
  left_tanget.magnitude = f(t, left_tangent.magnitude);
  right_tangent.magnitude = f(t, right_tangent.magnitude);
  Point offset(position + t * pdirection, left_tanget, right_tangent);
  const double tn = t / Vec2f(left_tanget.magnitude, right_tangent.magnitude).euclidean_norm();
  return offset.flattened(std::clamp(tn, 0.0, 1.0));
}

std::vector<Point> Point::offset(const double t,
                                 const std::vector<Point>& points, const bool is_closed)
{
  const auto n = points.size();
  if (n >= 2) {
    std::vector<Point> off_points;
    off_points.reserve(n);
    const auto* left = is_closed ? &points.back() : nullptr;
    off_points.push_back(points[0].offset(t, left, &points[1]));

    for (std::size_t i = 1; i < n-1; ++i) {
      off_points.push_back(points[i].offset(t, &points[i-1], &points[i+1]));
    }

    const auto* right = is_closed ? &points.front() : nullptr;
    off_points.push_back(points[n-1].offset(t, &points[n-2], right));
    return off_points;
  } else if (n == 1) {
    return { points[0].offset(t, nullptr, nullptr) };
  } else {
    return {};
  }
}

bool Point::operator<(const Point& point) const
{
  if (position == point.position) {
    if (left_tangent == point.left_tangent) {
      return right_tangent < point.right_tangent;
    } else {
      return left_tangent < point.left_tangent;
    }
  } else {
    return position < point.position;
  }
}

}  // namespace omm
