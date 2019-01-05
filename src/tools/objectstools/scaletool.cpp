#include "tools/objectstools/scaletool.h"
#include <memory>
#include "tools/handles/axishandle.h"
#include "tools/handles/particlehandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

arma::vec2 get_scale(const arma::vec2& pos, const arma::vec2& delta, const arma::vec2& direction)
{
  const auto factor = [](const double new_pos, const double delta, const double constraint) {
    const double old_pos = new_pos - delta;
    const double epsilon = 0.0001;
    if (std::abs(old_pos) < epsilon) {
      return 1.0;
    } else {
      double s = new_pos / old_pos;
      s = std::copysign(std::pow(std::abs(s), std::abs(constraint)), s);
      return s;
      if (std::abs(s) < 0.1) {
        return std::copysign(1.0, s);
      } else {
        return s;
      }
    }
  };
  const arma::vec old_pos = pos - delta;
  const arma::vec d = direction / arma::norm(direction);
  return arma::vec({ factor(pos(0), delta(0), d(0)), factor(pos(1), delta(1), d(1)) });
}

template<typename ItemT>
class ScaleAxisHandle : public omm::AxisHandle
{
public:
  ScaleAxisHandle(ItemT& tool) : m_tool(tool) { }
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    AxisHandle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const auto t = omm::ObjectTransformation().scaled(get_scale(pos, delta, m_direction));
      m_tool.transform_objects(t);
    }
  }

  void draw(omm::AbstractRenderer& renderer) const override
  {
    const omm::Point o(arma::vec2{ 0.0, 0.0 });
    const omm::Point tip(m_direction);
    const double r = 3.0;
    const omm::Point tl(m_direction + arma::vec2{-r,  r });
    const omm::Point bl(m_direction + arma::vec2{ r,  r });
    const omm::Point tr(m_direction + arma::vec2{-r, -r });
    const omm::Point br(m_direction + arma::vec2{ r, -r });

    renderer.draw_spline({ o, tip }, current_style());
    renderer.draw_spline({ tl, bl, br, tr, tl }, current_style());
  }

private:
  ItemT& m_tool;
};

template<typename ItemT>
class PointScaleHandle : public omm::ParticleHandle
{
public:
  PointScaleHandle(ItemT& tool) : m_tool(tool) {}
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    ParticleHandle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      const auto scale = get_scale(pos, delta, arma::vec2{ 1.0, 1.0 });
      const auto t = omm::ObjectTransformation().scaled(scale);
      m_tool.transform_objects(t);
    }
  }

private:
  ItemT& m_tool;
};

template<typename ItemT>
auto make_handles(ItemT& tool)
{
  using Status = omm::Handle::Status;

  // TODO scale handle close to object origin is inconvenient.
  // Replace the handle with some other kind of handle further away.
  auto point = std::make_unique<PointScaleHandle<ItemT>>(tool);

  auto x_axis = std::make_unique<ScaleAxisHandle<ItemT>>(tool);
  x_axis->set_style(Status::Active, omm::ContourStyle(omm::Color(1.0, 1.0, 1.0)));
  x_axis->set_style(Status::Hovered, omm::ContourStyle(omm::Color(1.0, 0.0, 0.0)));
  x_axis->set_style(Status::Inactive, omm::ContourStyle(omm::Color(1.0, 0.3, 0.3)));
  x_axis->set_direction({100, 0});

  auto y_axis = std::make_unique<ScaleAxisHandle<ItemT>>(tool);
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
  : ObjectsTool(scene, make_handles(*this))
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
  : ObjectsTool(scene, make_handles(*this))
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
