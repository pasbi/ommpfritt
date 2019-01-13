#pragma once

#include "tools/itemtools/transformationtool.h"
#include "objects/object.h"
#include "properties/floatproperty.h"
#include "tools/itemtools/positionvariant.h"
#include "tools/handles/selecthandle.h"

namespace omm
{

template<typename PositionVariant>
class SelectTool : public Tool
{
public:
  explicit SelectTool(Scene& scene)
    : Tool(scene)
    , position_variant(scene)
  {
    this->template add_property<FloatProperty>(RADIUS_PROPERTY_KEY, 20.0)
      .set_label(QObject::tr("radius").toStdString())
      .set_category(QObject::tr("tool").toStdString());
  }

  static constexpr auto RADIUS_PROPERTY_KEY = "radius";
  bool mouse_press(const arma::vec2& pos, const QMouseEvent& event) override;
  void transform_objects(const ObjectTransformation& transformation);
  void on_scene_changed() override;
  PositionVariant position_variant;
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
  void on_selection_changed() override;
  PointSelectHandle::TangentMode tangent_mode() const;
  std::unique_ptr<QMenu> make_context_menu(QWidget* parent) override;
};

}  // namespace omm
