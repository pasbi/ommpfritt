#pragma once

#include "objects/object.h"
#include "objects/instance.h"

namespace omm
{

class Property;
class Tag;
class Scene;

class Cloner : public Object
{
public:
  explicit Cloner(Scene* scene);
  void render(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() override;
  std::string type() const override;
  static constexpr auto TYPE = "Cloner";
  static constexpr auto COUNT_PROPERTY_KEY = "count";
  static constexpr auto CODE_PROPERTY_KEY = "code";
  std::unique_ptr<Object> clone() const override;
  virtual Flag flags() const override;
  std::unique_ptr<Object> convert() override;

private:
  std::vector<std::unique_ptr<Object>> make_clones();

};

}  // namespace omm