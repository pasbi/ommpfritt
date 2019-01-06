#include "tools/itemtools/movetool.h"
#include <memory>
#include "tools/handles/axishandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

template<typename ToolT>
auto make_handles(ToolT& tool)
{
  using Status = omm::Handle::Status;
  auto point = std::make_unique<omm::PointMoveHandle<ToolT>>(tool);

  auto x_axis = std::make_unique<omm::MoveAxisHandle<ToolT>>(tool);
  x_axis->set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
  x_axis->set_style(Status::Hovered, omm::ContourStyle(omm::Color(1.0, 0.0, 0.0)));
  x_axis->set_style(Status::Inactive, omm::ContourStyle(omm::Color(1.0, 0.3, 0.3)));
  x_axis->set_direction({100, 0});

  auto y_axis = std::make_unique<omm::MoveAxisHandle<ToolT>>(tool);
  y_axis->set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
  y_axis->set_style(Status::Hovered, omm::ContourStyle(omm::Color(0.0, 1.0, 0.0)));
  y_axis->set_style(Status::Inactive, omm::ContourStyle(omm::Color(0.3, 1.0, 0.3)));
  y_axis->set_direction({0, 100});

  std::vector<std::unique_ptr<omm::Handle>> handles;
  handles.push_back(std::move(point));
  handles.push_back(std::move(x_axis));
  handles.push_back(std::move(y_axis));
  return handles;
}

}  // namespace

namespace omm
{

ObjectMoveTool::ObjectMoveTool(Scene& scene)
  : TransformationTool(scene, make_handles(*this))
{
}

std::string ObjectMoveTool::type() const
{
  return TYPE;
}

QIcon ObjectMoveTool::icon() const
{
  return QIcon();
}

PointMoveTool::PointMoveTool(Scene& scene)
  : TransformationTool(scene, make_handles(*this))
{
}

std::string PointMoveTool::type() const
{
  return TYPE;
}

QIcon PointMoveTool::icon() const
{
  return QIcon();
}

}  // namespace omm
