#include "tools/itemtools/rotatetool.h"
#include <memory>
#include "tools/handles/circlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

template<typename ToolT>
auto make_handles(ToolT& tool)
{
  using Status = omm::Handle::Status;
  auto rh = std::make_unique<omm::RotateHandle<ToolT>>(tool);
  rh->set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
  rh->set_style(Status::Hovered, omm::ContourStyle(omm::Color(0.0, 0.0, 1.0)));
  rh->set_style(Status::Inactive, omm::ContourStyle(omm::Color(0.3, 0.3, 1.0)));
  rh->set_radius(100);

  std::vector<std::unique_ptr<omm::Handle>> handles;
  handles.push_back(std::move(rh));
  return handles;
}

}  // namespace

namespace omm
{

ObjectRotateTool::ObjectRotateTool(Scene& scene)
  : TransformationTool(scene, make_handles(*this))
{
}

std::string ObjectRotateTool::type() const
{
  return TYPE;
}

QIcon ObjectRotateTool::icon() const
{
  return QIcon();
}

PointRotateTool::PointRotateTool(Scene& scene)
  : TransformationTool(scene, make_handles(*this))
{
}

std::string PointRotateTool::type() const
{
  return TYPE;
}

QIcon PointRotateTool::icon() const
{
  return QIcon();
}

}  // namespace omm
