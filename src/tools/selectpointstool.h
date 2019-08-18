#pragma once

#include "tools/selecttool.h"
#include "tools/handles/moveaxishandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scalebandhandle.h"
#include "tools/handles/particlehandle.h"
#include "commands/pointstransformationcommand.h"

namespace omm
{

class SelectPointsBaseTool : public AbstractSelectTool
{
public:
  explicit SelectPointsBaseTool(Scene& scene);
  static constexpr auto TANGENT_MODE_PROPERTY_KEY = "tangent_mode";
  PointSelectHandle::TangentMode tangent_mode() const;
  std::unique_ptr<QMenu> make_context_menu(QWidget* parent) override;
  void on_selection_changed() override;
  void transform_objects(ObjectTransformation t) override;
  bool mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force) override;

  bool has_transformation() const override;

  template<typename ToolT> static void make_handles(ToolT& tool, bool force_subhandles = false)
  {
    tool.handles.push_back(std::make_unique<ScaleBandHandle<ToolT>>(tool));
    tool.handles.push_back(std::make_unique<RotateHandle<ToolT>>(tool));
    tool.handles.push_back(std::make_unique<MoveParticleHandle<ToolT>>(tool));

    static constexpr auto X = MoveAxisHandleDirection::X;
    static constexpr auto Y = MoveAxisHandleDirection::Y;
    tool.handles.push_back(std::make_unique<MoveAxisHandle<ToolT, X>>(tool));
    tool.handles.push_back(std::make_unique<MoveAxisHandle<ToolT, Y>>(tool));

    for (auto* path : type_cast<Path*>(tool.scene.template item_selection<Object>())) {
      tool.handles.reserve(tool.handles.size() + path->points().size());
      for (auto* point : path->points_ref()) {
        auto handle = std::make_unique<PointSelectHandle>(tool, *path, *point);
        handle->force_draw_subhandles = force_subhandles;
        tool.handles.push_back(std::move(handle));
      }
    }
  }

  BoundingBox bounding_box() const;
  void transform_objects_absolute(ObjectTransformation t);
  bool modifies_points() const override;

protected:
  Vec2f selection_center() const override;
  PointsTransformationCommand::Map m_initial_points;

private:
  std::set<Path*> m_paths;

};

class SelectPointsTool : public SelectPointsBaseTool
{
public:
  using SelectPointsBaseTool::SelectPointsBaseTool;
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectPointsTool");

protected:
  void on_scene_changed() override;
};

}  // namespace
