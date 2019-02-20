#pragma once

#include "objects/object.h"

namespace omm
{

class Empty : public Object
{
public:
  using Object::Object;
  void render(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() override;
  std::string type() const override;
  std::unique_ptr<Object> clone() const override;
  static constexpr auto TYPE = "Empty";

};

std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm
