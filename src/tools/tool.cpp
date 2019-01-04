#include "tools/tool.h"

#include <memory>
#include "renderers/abstractrenderer.h"
#include "objects/object.h"
#include "properties/optionsproperty.h"

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

bool Tool::mouse_move(const arma::vec2& delta, const arma::vec2& pos)
{
  bool hit_something = false;
  if (selection().size() > 0) {
    for (auto&& handle : handles) {
      handle->mouse_move(delta, pos, !hit_something);
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

bool Tool::mouse_press(const arma::vec2& pos)
{
  if (selection().size() > 0) {
    for (auto&& handle : handles) {
      handle->mouse_press(pos);
      if (handle->status() == Handle::Status::Active) {
        return true;
      }
    }
  }
  return false;
}

void Tool::mouse_release()
{
  if (selection().size() > 0) {
    for (auto&& handle : handles) {
      handle->mouse_release();
    }
  }
}

void Tool::set_selection(const std::set<Object*>& objects)
{
  m_selection = objects;
}

const std::set<Object*> Tool::selection() const
{
  return m_selection;
}

void Tool::draw(AbstractRenderer& renderer) const
{
  for (auto&& handle : handles) {
    handle->draw(renderer);
  }
}


}  // namespace omm
