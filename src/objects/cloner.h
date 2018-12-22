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
  explicit Cloner();
  void render(AbstractRenderer& renderer, const Style& style) const override;
  BoundingBox bounding_box() const override;
  std::string type() const override;
  static constexpr auto TYPE = "Cloner";
  static constexpr auto COUNT_PROPERTY_KEY = "count";
  static constexpr auto CODE_PROPERTY_KEY = "code";

private:
  void render_instances(AbstractRenderer& renderer, const Style& style) const;
  void render_copies(AbstractRenderer& renderer, const Style& style) const;
};

}  // namespace omm