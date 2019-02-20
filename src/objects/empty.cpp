#include "objects/empty.h"
#include "properties/optionsproperty.h"

namespace omm
{

void Empty::render(AbstractRenderer&, const Style&) { return; }
BoundingBox Empty::bounding_box() { return BoundingBox({ arma::vec2{0, 0} }); }
std::string Empty::type() const { return TYPE; }
std::unique_ptr<Object> Empty::clone() const { return std::make_unique<Empty>(*this); }

}  // namespace omm
