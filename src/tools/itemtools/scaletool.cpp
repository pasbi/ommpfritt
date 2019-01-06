#include "tools/itemtools/scaletool.h"
#include <memory>
#include "tools/handles/axishandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

template<typename ItemT>
auto make_handles(ItemT& tool)
{
  using Status = omm::Handle::Status;

  // TODO scale handle close to object origin is inconvenient.
  // Replace the handle with some other kind of handle further away.
  auto point = std::make_unique<omm::PointScaleHandle<ItemT>>(tool);

  auto x_axis = std::make_unique<omm::ScaleAxisHandle<ItemT>>(tool);
  x_axis->set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
  x_axis->set_style(Status::Hovered, omm::ContourStyle(omm::Color(1.0, 0.0, 0.0)));
  x_axis->set_style(Status::Inactive, omm::ContourStyle(omm::Color(1.0, 0.3, 0.3)));
  x_axis->set_direction({100, 0});

  auto y_axis = std::make_unique<omm::ScaleAxisHandle<ItemT>>(tool);
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

ObjectScaleTool::ObjectScaleTool(Scene& scene)
  : TransformationTool(scene, make_handles(*this))
{

}

std::string ObjectScaleTool::type() const
{
  return TYPE;
}

QIcon ObjectScaleTool::icon() const
{
  return QIcon();
}

PointScaleTool::PointScaleTool(Scene& scene)
  : TransformationTool(scene, make_handles(*this))
{

}

std::string PointScaleTool::type() const
{
  return TYPE;
}

QIcon PointScaleTool::icon() const
{
  return QIcon();
}

}  // namespace omm
