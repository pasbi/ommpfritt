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
  enum class Mode { Object, Path };
  explicit Mirror(Scene* scene);
  Mirror(const Mirror& other);
  void draw_object(Painter& renderer, const Style& style, Painter::Options options) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Mirror");
  static constexpr auto DIRECTION_PROPERTY_KEY = "direction";
  static constexpr auto AS_PATH_PROPERTY_KEY = "as_path";
  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";
  static constexpr auto IS_INVERTED_PROPERTY_KEY = "inverted";

  virtual Flag flags() const override;
  std::unique_ptr<Object> convert() const override;
  void update() override;

protected:
  void on_property_value_changed(Property* property) override;
  void on_child_added(Object &child) override;
  void on_child_removed(Object &child) override;

private:
  std::unique_ptr<Object> m_reflection;
  ObjectTransformation get_mirror_t() const;
  void polish();

};

}  // namespace omm
