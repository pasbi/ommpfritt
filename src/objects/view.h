#pragma once

#include "objects/object.h"

namespace omm
{
class View : public Object
{
public:
  explicit View(Scene* scene);
  explicit View(const View& other);
  View(View&&) = delete;
  View& operator=(View&&) = delete;
  View& operator=(const View&) = delete;
  ~View() override = default;

  void draw_handles(Painter& renderer) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;
  void draw_object(Painter&, const Style&, const PainterOptions&) const override;
  static constexpr auto SIZE_PROPERTY_KEY = "size";
  static constexpr auto TO_VIEWPORT_PROPERTY_KEY = "to-viewport";
  static constexpr auto FROM_VIEWPORT_PROPERTY_KEY = "from-viewport";
  static constexpr auto OUTPUT_VIEW_PROPERTY_KEY = "output";
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "View");
  void on_property_value_changed(Property* property) override;
  Flag flags() const override;

private:
  void from_viewport();
  void to_viewport();
  void make_output_unique();
};

}  // namespace omm
