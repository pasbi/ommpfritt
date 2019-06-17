#pragma once

#include "tools/selecttool.h"
#include "commands/objectstransformationcommand.h"

namespace omm
{

class SelectObjectsTool : public AbstractSelectTool
{
public:
  explicit SelectObjectsTool(Scene& scene);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectObjectsTool");
  void transform_objects(ObjectTransformation t) override;
  static constexpr auto TRANSFORMATION_MODE_KEY = "transformation_mode";
  void on_scene_changed() override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force) override;
  BoundingBox bounding_box() const;

protected:
  bool has_transformation() const override;
  Vec2f selection_center() const override;
  ObjectsTransformationCommand::Map m_initial_transformations;

};

}  // namespace omm
