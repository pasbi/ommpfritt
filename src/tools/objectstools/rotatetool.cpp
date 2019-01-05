#include "tools/objectstools/rotatetool.h"
#include <memory>
#include "tools/handles/circlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

template<typename ItemT>
class RotateHandle : public omm::CircleHandle
{
public:
  RotateHandle(ItemT& tool) : m_tool(tool) { }

  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    Handle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const arma::vec2 origin = pos - delta;
      double origin_angle = atan2(origin(1), origin(0));
      double pos_angle = atan2(pos(1), pos(0));
      m_tool.transform_objects(omm::ObjectTransformation().rotated(pos_angle - origin_angle));
    }
  }

private:
  ItemT& m_tool;
};

template<typename ItemT>
auto make_handles(ItemT& tool)
{
  using Status = omm::Handle::Status;
  auto rh = std::make_unique<RotateHandle<ItemT>>(tool);
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
  : ObjectsTool(scene, make_handles(*this))
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
  : ObjectsTool(scene, make_handles(*this))
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
