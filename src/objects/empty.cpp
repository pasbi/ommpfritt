#include "objects/empty.h"
#include "properties/optionsproperty.h"

namespace omm
{

BoundingBox Empty::bounding_box() const { return BoundingBox({ Vec2f(0, 0) }); }
std::string Empty::type() const { return TYPE; }
std::unique_ptr<Object> Empty::clone() const { return std::make_unique<Empty>(*this); }

}  // namespace omm
