#pragma once

#include "objects/evaluatable.h"

namespace omm
{

PointOnCubic Evaluatable::evaluate(const double t)
{
  return cubic_segments().evaluate(t);
}

double Evaluatable::length()
{
  return cubic_segments().length();
}


}  // namespace omm
