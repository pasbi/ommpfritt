#include "path/edge.h"
#include "path/pathpoint.h"
#include "geometry/point.h"

namespace omm
{

QString Edge::label() const
{
  const auto* separator = flipped ? "++" : "--";
  return start_point()->debug_id() + separator + end_point()->debug_id();
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
