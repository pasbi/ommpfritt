#include "tools/tool.h"

#include <memory>
#include "renderers/abstractrenderer.h"
#include "objects/object.h"
#include "properties/optionsproperty.h"
#include "scene/scene.h"

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

}  // namespace

namespace omm
{

Tool::Tool(Scene& scene)
  : scene(scene)
{
}

bool Tool::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e)
{
  for (auto&& handle : handles) {
    handle->mouse_move(delta, pos, e);
    switch (handle->status()) {
    case Handle::Status::Active:
      return true;
      break;
    case Handle::Status::Hovered:
    case Handle::Status::Inactive:
      break;
    }
  }
  return false;
}

bool Tool::mouse_press(const arma::vec2& pos, const QMouseEvent& e)
{
  // `std::any_of` does not *require* to use short-circuit-logic. However, here it is mandatory,
  // so don't use `std::any_of`.
  for (auto&& handle : handles) {
    if (handle->mouse_press(pos, e)) {
      return true;
    }
  }
  return false;
}

void Tool::
mouse_release(const arma::vec2& pos, const QMouseEvent& e)
{
  for (auto&& handle : handles) {
    handle->mouse_release(pos, e);
  }
}

void Tool::draw(AbstractRenderer& renderer) const
{
  for (auto&& handle : handles) {
    handle->draw(renderer);
  }
}

std::unique_ptr<QMenu> Tool::make_context_menu(QWidget* parent) { return nullptr; }

void Tool::on_selection_changed() {}
void Tool::on_scene_changed() {}


}  // namespace omm
