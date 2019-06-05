#pragma once

#include "tools/selecttool.h"
#include "tools/handles/moveaxishandle.h"
#include "tools/handles/rotatehandle.h"
#include "tools/handles/scalebandhandle.h"
#include "tools/handles/particlehandle.h"

namespace omm
{

class SelectPointsTool : public AbstractSelectTool
{
public:
  explicit SelectPointsTool(Scene& scene);
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "SelectPointsTool");
  static constexpr auto TANGENT_MODE_PROPERTY_KEY = "tangent_mode";
  PointSelectHandle::TangentMode tangent_mode() const;
  std::unique_ptr<QMenu> make_context_menu(QWidget* parent) override;
  void on_selection_changed() override;
  Command *transform_objects(ObjectTransformation t, const bool tool_space) override;
  void on_scene_changed() override;
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


    for (auto* path : tool.paths()) {
      tool.handles.reserve(tool.handles.size() + path->points().size());
      for (auto* point : path->points_ref()) {
        auto handle = std::make_unique<PointSelectHandle>(tool, *path, *point);
        handle->force_draw_subhandles = force_subhandles;
        tool.handles.push_back(std::move(handle));
      }
    }
  }

protected:
  std::set<Point*> selected_points() const;
  Vec2f selection_center() const;
  std::set<Path*> paths() const;

};

}  // namespace
