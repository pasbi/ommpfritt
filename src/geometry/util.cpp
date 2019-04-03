#include "geometry/util.h"
#include <cmath>

namespace omm
{

Vec2f get_scale(const Vec2f& pos, const Vec2f& delta, const Vec2f& direction)
{
  const auto factor = [](const double new_pos, const double delta, const double constraint) {
    const double old_pos = new_pos - delta;
    const double epsilon = 0.0001;
    if (std::abs(old_pos) < epsilon) {
      return 1.0;
    } else {
      double s = new_pos / old_pos;
      s = std::pow(s, std::abs(constraint));
      return s;
      if (std::abs(s) < 0.1) {
        return std::copysign(1.0, s);
      } else {
        return s;
      }
    }
  };
  const Vec2f old_pos = pos - delta;
  const Vec2f d = direction / direction.euclidean_norm();
  return Vec2f(factor(pos.x, delta.x, d.x), factor(pos.y, delta.y, d.y));
}

}  // namespace omm
