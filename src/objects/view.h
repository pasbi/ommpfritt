#pragma once

#include "objects/object.h"

namespace omm
{

class View : public Object
{
public:
  explicit View(Scene* scene);
  void draw_handles(AbstractRenderer& renderer) override;
  BoundingBox bounding_box() override;
  std::string type() const override;
  std::unique_ptr<Object> clone() const override;
  static constexpr auto SIZE_PROPERTY_KEY = "size";
  static constexpr auto TO_VIEWPORT_PROPERTY_KEY = "to-viewport";
  static constexpr auto FROM_VIEWPORT_PROPERTY_KEY = "from-viewport";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "View");
  void on_property_value_changed(Property& property) override;
  Flag flags() const override;
  ObjectTransformation transformation() const;

private:
  void from_viewport();
  void to_viewport();
};

}  // namespace omm
