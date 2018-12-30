#include "tools/movetool.h"
#include <memory>
#include "tools/handles/axishandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

class MoveAxisHandle : public omm::AxisHandle
{
public:
  MoveAxisHandle(omm::MoveTool& tool) : m_tool(tool) { }

  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    Handle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const auto t = omm::ObjectTransformation().translated(project_onto_axis(delta));
      m_tool.transform_objects(t);
    }
  }

  void draw(omm::AbstractRenderer& renderer) const override
  {
    const double magnitude = arma::norm(m_direction);
    const double argument = std::atan2(m_direction[1], m_direction[0]);

    const auto o = omm::Point(arma::vec2{ 0, 0 });
    const auto tip = omm::Point(m_direction);
    const auto right = omm::Point(argument-0.1, magnitude*0.9);
    const auto left = omm::Point(argument+0.1, magnitude*0.9);

    renderer.draw_spline({ o, tip }, current_style());
    renderer.draw_spline({ left, tip, right, left }, current_style());
  }

private:
  omm::MoveTool& m_tool;
};

auto make_handles(omm::MoveTool& tool)
{
  const auto make_style = [](const omm::Color& color) {
    omm::Style style;
    style.property(omm::Style::PEN_IS_ACTIVE_KEY).set(true);
    style.property(omm::Style::BRUSH_IS_ACTIVE_KEY).set(true);
    style.property(omm::Style::PEN_COLOR_KEY).set(color);
    style.property(omm::Style::BRUSH_COLOR_KEY).set(color);
    style.property(omm::Style::PEN_WIDTH_KEY).set(2.0);
    return style;
  };

  using Status = omm::Handle::Status;
  auto x_axis = std::make_unique<MoveAxisHandle>(tool);
  x_axis->set_style(Status::Active, make_style(omm::Color(1.0, 1.0, 1.0)));
  x_axis->set_style(Status::Hovered, make_style(omm::Color(1.0, 0.0, 0.0)));
  x_axis->set_style(Status::Inactive, make_style(omm::Color(1.0, 0.3, 0.3)));
  x_axis->set_direction({100, 0});

  auto y_axis = std::make_unique<MoveAxisHandle>(tool);
  y_axis->set_style(Status::Active, make_style(omm::Color(1.0, 1.0, 1.0)));
  y_axis->set_style(Status::Hovered, make_style(omm::Color(0.0, 1.0, 0.0)));
  y_axis->set_style(Status::Inactive, make_style(omm::Color(0.3, 1.0, 0.3)));
  y_axis->set_direction({0, 100});

  std::vector<std::unique_ptr<omm::Handle>> handles;
  handles.push_back(std::move(x_axis));
  handles.push_back(std::move(y_axis));
  return handles;
}

}  // namespace

namespace omm
{

MoveTool::MoveTool(Scene& scene)
  : ObjectTransformationTool(scene, make_handles(*this))
{

}

std::string MoveTool::type() const
{
  return TYPE;
}

QIcon MoveTool::icon() const
{
  return QIcon();
}

}  // namespace omm
