#include "objects/empty.h"

namespace omm
{

BoundingBox Empty::bounding_box(const ObjectTransformation &transformation) const
{
  const Vec2f o = transformation.apply_to_position(Vec2f::o());
  return BoundingBox({ o });
}
QString Empty::type() const { return TYPE; }

}  // namespace omm
