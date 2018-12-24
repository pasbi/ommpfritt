#pragma once

#include "objects/object.h"

namespace omm
{

class Property;
class Tag;
class Scene;

class Instance : public Object
{
public:
  explicit Instance(Scene& scene);
  void render(AbstractRenderer& renderer, const Style& style) const override;
  BoundingBox bounding_box() const override;
  std::string type() const override;
  static constexpr auto TYPE = "Instance";
  static constexpr auto REFERENCE_PROPERTY_KEY = "reference";

private:
  Object* referenced_object() const;
};

}  // namespace omm
