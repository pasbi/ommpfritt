#include "objects/empty.h"
#include "properties/optionsproperty.h"

namespace omm
{

BoundingBox Empty::bounding_box(const ObjectTransformation &transformation) const
{
  const Vec2f o = global_transformation(false).apply_to_position(Vec2f::o());
  return BoundingBox({ o });
}
std::string Empty::type() const { return TYPE; }
std::unique_ptr<Object> Empty::clone() const { return std::make_unique<Empty>(*this); }

}  // namespace omm
