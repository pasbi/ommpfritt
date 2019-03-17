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

Tool::Tool(Scene& scene) : scene(scene) { }
ObjectTransformation Tool::transformation() const { return ObjectTransformation(); }

bool Tool::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e)
{
  for (auto&& handle : handles) {
    if (handle->is_enabled()) {
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
  }
  return false;
}

bool Tool::mouse_press(const arma::vec2& pos, const QMouseEvent& e, bool force)
{
  // `std::any_of` does not *require* to use short-circuit-logic. However, here it is mandatory,
  // so don't use `std::any_of`.
  for (auto&& handle : handles) {
    if (handle->is_enabled() && handle->mouse_press(pos, e, force)) {
      return true;
    }
  }
  return false;
}

void Tool::
mouse_release(const arma::vec2& pos, const QMouseEvent& e)
{
  for (auto&& handle : handles) {
    if (handle->is_enabled()) {
      handle->mouse_release(pos, e);
    }
  }
}

void Tool::draw(AbstractRenderer& renderer) const
{
  renderer.set_category(AbstractRenderer::Category::Handles);
  const ObjectTransformation transformation = this->transformation();
  for (auto&& handle : handles) {
    if (handle->is_enabled()) {
      if (handle->transform_in_tool_space) {
        renderer.push_transformation(transformation);
        handle->draw(renderer);
        renderer.pop_transformation();
      } else {
        handle->draw(renderer);
      }
    }
  }
}

std::unique_ptr<QMenu> Tool::make_context_menu(QWidget* parent) { return nullptr; }
bool Tool::has_transformation() const { return false; }
void Tool::on_selection_changed() {}
void Tool::on_scene_changed() {}
void Tool::transform_objects(ObjectTransformation t, const bool tool_space) {}
AbstractPropertyOwner::Flag Tool::flags() const { return Flag::None; }

}  // namespace omm
