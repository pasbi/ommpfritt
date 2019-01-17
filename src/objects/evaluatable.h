#pragma once

#include "geometry/cubic.h"

namespace omm
{

class Evaluatable
{
public:
  PointOnCubic evaluate(const double t);
  double length();

protected:
  virtual Cubics cubic_segments() = 0;
};

}  // namespace omm
