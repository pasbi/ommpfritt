#pragma once

#include "tools/selecttool.h"
#include "tools/handles/moveaxishandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scaleaxishandle.h"
#include "tools/handles/scalebandhandle.h"
#include "tools/handles/particlehandle.h"
#include "commands/pointstransformationcommand.h"

namespace omm
{

class TransformPointsHelper
{
public:
  explicit TransformPointsHelper(Space space);
  std::unique_ptr<PointsTransformationCommand> make_command(const ObjectTransformation& t) const;
  void update(const std::set<Path *> &paths);
  void update();
  bool is_empty() const { return m_initial_points.size() == 0; }

private:
  PointsTransformationCommand::Map m_initial_points;
  std::set<Path*> m_paths;
  const Space m_space;
};

class SelectPointsBaseTool : public AbstractSelectTool
{
public:
  explicit SelectPointsBaseTool(Scene& scene);
  static constexpr auto TANGENT_MODE_PROPERTY_KEY = "tangent_mode";
  static constexpr auto BOUNDING_BOX_MODE_PROPERTY_KEY = "bounding_box_mode";
  enum class BoundingBoxMode { IncludeTangents, ExcludeTangents, None };
  PointSelectHandle::TangentMode tangent_mode() const;
  std::unique_ptr<QMenu> make_context_menu(QWidget* parent) override;
  void transform_objects(ObjectTransformation t) override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event) override;

  bool has_transformation() const override;

  template<typename ToolT> static void make_handles(ToolT& tool, bool force_subhandles = false)
  {
//    tool.handles.push_back(std::make_unique<ScaleBandHandle<ToolT>>(tool));
//    tool.handles.push_back(std::make_unique<RotateHandle<ToolT>>(tool));

//    static constexpr auto X = AxisHandleDirection::X;
//    static constexpr auto Y = AxisHandleDirection::Y;
//    tool.handles.push_back(std::make_unique<MoveAxisHandle<ToolT, X>>(tool));
//    tool.handles.push_back(std::make_unique<MoveAxisHandle<ToolT, Y>>(tool));
//    tool.handles.push_back(std::make_unique<ScaleAxisHandle<ToolT, X>>(tool));
//    tool.handles.push_back(std::make_unique<ScaleAxisHandle<ToolT, Y>>(tool));

//    tool.handles.push_back(std::make_unique<MoveParticleHandle<ToolT>>(tool));

//    for (auto* path : type_cast<Path*>(tool.scene()->template item_selection<Object>())) {
//      tool.handles.reserve(tool.handles.size() + path->points().size());
//      for (auto* point : path->points_ref()) {
//        auto handle = std::make_unique<PointSelectHandle>(tool, *path, *point);
//        handle->force_draw_subhandles = force_subhandles;
//        tool.handles.push_back(std::move(handle));
//      }
//    }
  }

  BoundingBox bounding_box() const;
  void transform_objects_absolute(ObjectTransformation t);
  bool modifies_points() const override;
  void on_property_value_changed(Property* property) override;

protected:
  Vec2f selection_center() const override;

private:
  TransformPointsHelper m_transform_points_helper;
};

class SelectPointsTool : public SelectPointsBaseTool
{
public:
  using SelectPointsBaseTool::SelectPointsBaseTool;
  QString type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectPointsTool");

protected:
  void reset() override;
};

}  // namespace
