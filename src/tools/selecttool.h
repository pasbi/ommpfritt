#pragma once

#include "tools/tool.h"
#include "objects/object.h"
#include "properties/floatproperty.h"
#include "tools/positionvariant.h"

namespace omm
{

template<typename PositionVariant>
class SelectTool : public Tool
{
public:
  explicit SelectTool(Scene& scene)
    : Tool(scene)
    , m_position_variant(scene)
  {
    add_property<FloatProperty>(RADIUS_PROPERTY_KEY, 20.0)
      .set_label(QObject::tr("radius").toStdString())
      .set_category(QObject::tr("tool").toStdString());
  }

  static constexpr auto RADIUS_PROPERTY_KEY = "radius";
  bool mouse_press(const arma::vec2& pos) override;
  void activate() override;

private:
  PositionVariant m_position_variant;

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
  using SelectTool::SelectTool;
  std::string type() const override;
  QIcon icon() const override;
  static constexpr auto TYPE = "SelectPointsTool";
};

}  // namespace omm
