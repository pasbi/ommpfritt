#include "geometry/point.h"

namespace omm
{

Point::Point(const arma::vec2& position)
  : position(position)
  , left_tangent(arma::vec2 { 0, 0 })
  , right_tangent(arma::vec2 { 0, 0 })
{
}

arma::vec2 Point::left_position() const
{
  return position + left_tangent;
}

arma::vec2 Point::right_position() const
{
  return position + right_tangent;
}

}  // namespace omm
