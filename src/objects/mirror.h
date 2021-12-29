#pragma once

#include "objects/object.h"
#include <Qt>
#include <variant>

namespace omm
{
class Property;
class Tag;
class Scene;

class Mirror : public Object
{
public:
  enum class Direction { Horizontal, Vertical, Both };
  enum class Mode { Object, Path };
  explicit Mirror(Scene* scene);
  Mirror(const Mirror& other);
  Mirror(Mirror&&) = delete;
  Mirror& operator=(Mirror&&) = delete;
  Mirror& operator=(const Mirror&) = delete;
  ~Mirror() override = default;

  void draw_object(Painter& renderer,
                   const Style& style,
                   const PainterOptions& options) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Mirror");
  static constexpr auto DIRECTION_PROPERTY_KEY = "direction";
  static constexpr auto AS_PATH_PROPERTY_KEY = "as_path";
  static constexpr auto TOLERANCE_PROPERTY_KEY = "eps";

  PathVector compute_path_vector() const override;
  std::unique_ptr<Object> convert(bool& keep_children) const override;
  void update() override;

protected:
  void on_property_value_changed(Property* property) override;
  void on_child_added(Object& child) override;
  void on_child_removed(Object& child) override;

private:
  std::unique_ptr<Object> m_reflection;
  void polish();
  void update_object_mode();
  void update_path_mode();
  void update_property_visibility();
};

}  // namespace omm
