#pragma once

#include "objects/object.h"
#include <Qt>
#include "logging.h"

namespace omm
{

class Property;
class Tag;
class Scene;

class Instance : public Object
{
public:
  explicit Instance(Scene* scene);
  Instance(const Instance& other);
  void draw_object(Painter& renderer, const Style& default_style) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Instance");
  static constexpr auto REFERENCE_PROPERTY_KEY = "reference";
  static constexpr auto COMBINE_STYLES_PROPERTY_KEY = "combine-styles";
  std::unique_ptr<Object> clone() const override;
  std::unique_ptr<Object> convert() const override;
  Flag flags() const override;
  void post_create_hook() override;

protected:
  void on_property_value_changed(Property *property) override;

private:
  Object* referenced_object() const;
};

}  // namespace omm
