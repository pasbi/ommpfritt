#pragma once

#include "tools/handles/particlehandle.h"
#include "tools/selecttool.h"
#include "tools/handles/pointselecthandle.h"
#include <memory>

namespace omm
{

class TransformPointsHelper;

class SelectPointsBaseTool : public AbstractSelectTool
{
public:
  explicit SelectPointsBaseTool(Scene& scene);
  ~SelectPointsBaseTool() override;
  SelectPointsBaseTool(const SelectPointsBaseTool&) = delete;
  SelectPointsBaseTool(SelectPointsBaseTool&&) = delete;
  SelectPointsBaseTool& operator=(const SelectPointsBaseTool&) = delete;
  SelectPointsBaseTool& operator=(SelectPointsBaseTool&&) = delete;
  static constexpr auto TANGENT_MODE_PROPERTY_KEY = "tangent_mode";
  static constexpr auto BOUNDING_BOX_MODE_PROPERTY_KEY = "bounding_box_mode";
  enum class BoundingBoxMode {IncludeTangents, ExcludeTangents, None};
  PointSelectHandle::TangentMode tangent_mode() const;
  std::unique_ptr<QMenu> make_context_menu(QWidget* parent) override;
  void transform_objects(ObjectTransformation t) override;

  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool allow_clear);

  BoundingBox bounding_box() const;
  void transform_objects_absolute(ObjectTransformation t);
  void on_property_value_changed(Property* property) override;
  SceneMode scene_mode() const override;

  void reset() override;

protected:
  Vec2f selection_center() const override;

private:
  void make_handles();
  std::unique_ptr<TransformPointsHelper> m_transform_points_helper;
};

}  // namespace omm
