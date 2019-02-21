#pragma once

#include "objects/object.h"
#include <Qt>

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
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Empty", "Empty");

};

std::ostream& operator<<(std::ostream& ostream, const Object& object);

}  // namespace omm
