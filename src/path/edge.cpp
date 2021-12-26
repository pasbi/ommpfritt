#include "path/edge.h"
#include "path/pathpoint.h"
#include "geometry/point.h"

namespace omm
{

QString Edge::label() const
{
  const auto* separator = flipped ? "++" : "--";
  return QString{"%1%2%3"}.arg(start_point()->index()).arg(separator).arg(end_point()->index());
}

Point Edge::start_geometry() const
{
  const auto g = start_point()->geometry();
//  return flipped ? g.flipped() : g;
  return flipped ?  g : g.flipped();
}

Point Edge::end_geometry() const
{
  const auto g = end_point()->geometry();
//  return flipped ? g.flipped() : g;
  return flipped ?  g : g.flipped();
}

PathPoint* Edge::start_point() const
{
  return flipped ? b : a;
}

PathPoint* Edge::end_point() const
{
  return flipped ? a : b;
}

}  // namespace omm
