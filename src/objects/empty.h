#pragma once

#include "objects/object.h"

namespace omm
{

class Empty : public Object
{
public:
  explicit Empty();
  void render(AbstractRenderer& renderer) const override;
  BoundingBox bounding_box() const override;
  std::string type() const override;
};

std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm