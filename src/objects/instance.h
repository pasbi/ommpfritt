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
  explicit Instance(Scene* scene);
  void render(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() override;
  std::string type() const override;
  static constexpr auto TYPE = "Instance";
  static constexpr auto REFERENCE_PROPERTY_KEY = "reference";
  std::unique_ptr<Object> clone() const override;

private:
  Object* referenced_object() const;
};

}  // namespace omm
