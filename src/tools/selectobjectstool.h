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

  /**
   * @brief transform_objects transforms the selected objects with given transformation @code t.
   * @param t the transformation in viewport-space.
   */
  void transform_objects(ObjectTransformation t) override;
  static constexpr auto TRANSFORMATION_MODE_KEY = "transformation_mode";
  void reset() override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force) override;
  BoundingBox bounding_box() const;

protected:
  bool has_transformation() const override;
  Vec2f selection_center() const override;
  ObjectsTransformationCommand::Map m_initial_transformations;

};

}  // namespace omm
