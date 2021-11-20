#pragma once

#include "tools/selecttool.h"

namespace omm
{

class TransformObjectsHelper;

class SelectObjectsTool : public AbstractSelectTool
{
public:
  explicit SelectObjectsTool(Scene& scene);
  ~SelectObjectsTool() override;
  SelectObjectsTool(const SelectObjectsTool&) = delete;
  SelectObjectsTool(SelectObjectsTool&&) = delete;
  SelectObjectsTool operator=(const SelectObjectsTool&) = delete;
  SelectObjectsTool operator=(SelectObjectsTool&&) = delete;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectObjectsTool");

  /**
   * @brief transform_objects transforms the selected objects with given transformation @code t.
   * @param t the transformation in viewport-space.
   */
  void transform_objects(ObjectTransformation t) override;
  static constexpr auto TRANSFORMATION_MODE_KEY = "transformation_mode";
  void reset() override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  void mouse_release(const Vec2f& pos, const QMouseEvent& event) override;
  BoundingBox bounding_box() const;
  SceneMode scene_mode() const override
  {
    return SceneMode::Object;
  }

protected:
  Vec2f selection_center() const override;
  std::unique_ptr<TransformObjectsHelper> m_transform_objects_helper;
};

}  // namespace omm
