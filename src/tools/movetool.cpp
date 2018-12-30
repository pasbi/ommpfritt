#include "tools/movetool.h"
#include <memory>
#include "tools/handles/axishandle.h"
#include "scene/scene.h"
#include "commands/objectstransformationcommand.h"

namespace
{

arma::vec2 get_global_position_mean(const std::set<omm::Object*>& objects)
{
  assert(objects.size() > 0);
  const auto add = [](const arma::vec2& accu, const omm::Object* object) -> arma::vec2 {
    return accu + object->global_transformation().translation();
  };
  const auto null = arma::vec2 {0.0, 0.0};
  return std::accumulate(objects.begin(), objects.end(), null, add) / objects.size();
}

void make(omm::Style& style, const omm::Color& color)
{

}

auto make_handles(omm::MoveTool& tool)
{
  const auto make_style = [](const omm::Color& color) {
    omm::Style style;
    style.property(omm::Style::PEN_IS_ACTIVE_KEY).set(true);
    style.property(omm::Style::BRUSH_IS_ACTIVE_KEY).set(false);
    style.property(omm::Style::PEN_COLOR_KEY).set(color);
    style.property(omm::Style::PEN_WIDTH_KEY).set(2.0);
    return style;
  };

  using Status = omm::Handle::Status;
  auto x_axis = std::make_unique<omm::MoveAxisHandle>(tool);
  x_axis->set_style(Status::Active, make_style(omm::Color(1.0, 1.0, 1.0)));
  x_axis->set_style(Status::Hovered, make_style(omm::Color(1.0, 0.0, 0.0)));
  x_axis->set_style(Status::Inactive, make_style(omm::Color(1.0, 0.3, 0.3)));
  x_axis->set_direction({100, 0});

  auto y_axis = std::make_unique<omm::MoveAxisHandle>(tool);
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
  : Tool(scene, make_handles(*this))
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

void MoveTool::transform_objects(const ObjectTransformation& transformation)
{
  const ObjectTransformation::Mat h = this->transformation().to_mat();
  const ObjectTransformation::Mat h_inv = h.i();
  const ObjectTransformation::Mat t = transformation.to_mat();

  const auto global = ObjectTransformation(h * t * h_inv);
  scene.submit<ObjectsTransformationCommand>(selection(), global);
}

bool MoveTool::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
{
  bool hit_something = false;
  if (selection().size() > 0) {
    const auto handle_local_pos = map_to_tool_local(pos);
    for (auto* handle : handles()) {
      const auto transformed_delta = transformation().inverted().apply_to_direction(delta);
      handle->mouse_move(transformed_delta, handle_local_pos, !hit_something);
      switch (handle->status()) {
      case Handle::Status::Active:
      case Handle::Status::Hovered:
        hit_something = true;
        break;
      case Handle::Status::Inactive:
        break;
      }
    }
  }
  return hit_something;
}

bool MoveTool::mouse_press(const arma::vec2& pos)
{
  if (selection().size() > 0) {
    const auto handle_local_pos = map_to_tool_local(pos);
    for (auto* handle : handles()) {
      handle->mouse_press(handle_local_pos);
      if (handle->status() == Handle::Status::Active) {
        return true;
      }
    }
  }
  return false;
}

void MoveTool::mouse_release()
{
  if (selection().size() > 0) {
    for (auto* handle : handles()) {
      handle->mouse_release();
    }
  }
}


}  // namespace omm
