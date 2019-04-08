#pragma once

#include "objects/object.h"
#include "objects/instance.h"
#include <Qt>

namespace omm
{

class Property;
class Tag;
class Scene;

class Mirror : public Object
{
public:
  enum class Direction { Horizontal, Vertical };
  explicit Mirror(Scene* scene);
  void render(AbstractRenderer& renderer, const Style& style) override;
  BoundingBox bounding_box() override;
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Mirror");
  static constexpr auto DIRECTION_PROPERTY_KEY = "direction";

  std::unique_ptr<Object> clone() const override;
  virtual Flag flags() const override;
  std::unique_ptr<Object> convert() override;

private:
  std::unique_ptr<Object> make_reflection();
  ObjectTransformation get_mirror_t() const;
};

}  // namespace omm
