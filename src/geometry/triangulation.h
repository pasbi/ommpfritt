#pragma once

#include <vector>
#include "geometry/point.h"
#include "geometry/triangle.h"

namespace omm
{

std::vector<Triangle> triangulate_delauney(const std::vector<Vec2f>& points);

}  // namespace omm
