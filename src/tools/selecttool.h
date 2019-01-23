#pragma once

#include "objects/object.h"
#include "tools/positionvariant.h"
#include "tools/handles/selecthandle.h"
#include "tools/tool.h"

namespace omm
{

template<typename PositionVariant>
class SelectTool : public Tool
{
public:
  explicit SelectTool(Scene& scene);
  static constexpr auto ALIGNMENT_PROPERTY_KEY = "alignment";
  static constexpr auto RADIUS_PROPERTY_KEY = "radius";
  bool mouse_press(const arma::vec2& pos, const QMouseEvent& event) override;
  void transform_objects(ObjectTransformation t, const bool tool_space) override;
  void on_scene_changed() override;
  PositionVariant position_variant;
  ObjectTransformation transformation() const override;
  bool has_transformation() const override;
};

class SelectObjectsTool : public SelectTool<ObjectPositions>
{
public:

  using SelectTool::SelectTool;
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "SelectObjectsTool";
};

class SelectPointsTool : public SelectTool<PointPositions>
{
public:
  explicit SelectPointsTool(Scene& scene);
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "SelectPointsTool";
  static constexpr auto TANGENT_MODE_PROPERTY_KEY = "tangent_mode";
  PointSelectHandle::TangentMode tangent_mode() const;
  std::unique_ptr<QMenu> make_context_menu(QWidget* parent) override;
  void on_selection_changed() override;
};

}  // namespace omm
