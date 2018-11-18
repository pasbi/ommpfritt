#include "objects/empty.h"

namespace omm
{

Empty::Empty()
{
}

void Empty::render(AbstractRenderer&, const Style&) const
{
  return;
}

BoundingBox Empty::bounding_box() const
{
  return BoundingBox({ arma::vec2{0, 0} });
}

std::string Empty::type() const
{
  return "Empty";
}


}  // namespace omm
