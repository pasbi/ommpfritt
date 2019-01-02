#pragma once

#include "objects/object.h"

namespace omm
{

class Scene;

class Ellipse : public Object
{
public:
  explicit Ellipse(Scene* scene);
  void render(AbstractRenderer& renderer, const Style& style) const override;
  BoundingBox bounding_box() const override;
  std::string type() const override;
  static constexpr auto TYPE = "Ellipse";
  std::unique_ptr<Object> clone() const override;
  std::unique_ptr<Object> convert() const override;
  Object::Flag flags() const override;
private:
};

}  // namespace omm
