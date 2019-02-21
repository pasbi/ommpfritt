#pragma once

#include <armadillo>
#include <Qt>

namespace omm
{

class OrientedPoint
{
public:
  OrientedPoint();
  OrientedPoint(const arma::vec2& pos, double rotation);
  const arma::vec2 position;
  const double rotation;
  const bool is_valid;
};

struct PolarCoordinates
{
  explicit PolarCoordinates(const double argument, const double magnitude);
  explicit PolarCoordinates(const arma::vec2& cartesian);
  explicit PolarCoordinates();
  arma::vec2 to_cartesian() const;
  void swap(PolarCoordinates& other);
  double argument;
  double magnitude;
  bool operator==(const PolarCoordinates& other) const;
  bool operator!=(const PolarCoordinates& other) const;
};

class Point
{
public:
  explicit Point(const arma::vec2& position);
  explicit Point(const OrientedPoint& op, const double tangent_length);
  Point();
  arma::vec2 position;
  arma::vec2 left_position() const;
  arma::vec2 right_position() const;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Point", "Point");
  bool is_selected = false;
  void swap(Point& other);

  PolarCoordinates left_tangent;
  PolarCoordinates right_tangent;

  Point smoothed(const Point& left_neighbor, const Point& right_neighbor) const;
  Point nibbed() const;
  bool operator==(const Point& point) const;
  bool operator!=(const Point& point) const;

};


constexpr PolarCoordinates to_polar(arma::vec2 cartesian);
constexpr arma::vec2 to_cartesian(const PolarCoordinates& polar);

std::ostream& operator<<(std::ostream& ostream, const PolarCoordinates& pc);
std::ostream& operator<<(std::ostream& ostream, const Point& pc);
std::ostream& operator<<(std::ostream& ostream, const Point* pc);

}  // namespace omm
