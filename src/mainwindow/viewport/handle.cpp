#include "mainwindow/viewport/handle.h"
#include <glog/logging.h>

#include "objects/object.h"
#include "renderers/style.h"
#include "mainwindow/viewport/styles.h"

namespace
{

void draw_arrow(omm::AbstractRenderer& renderer, const omm::Style& style)
{
  constexpr double LENGTH = 100;
  omm::Point center({0, 0});
  omm::Point tip({LENGTH, 0});
  renderer.draw_spline({ center, tip }, style);
}

}  // namespace

namespace omm
{

Handle::Handle(const std::set<Object*>& selection)
  : m_objects(selection)
{
}

Handle::Handle()
{
}

bool Handle::mouse_move(const arma::vec2& delta)
{
  return false;
}

bool Handle::mouse_press(const arma::vec2& pos)
{
  return false;
}

void Handle::mouse_release()
{
}

void Handle::draw(AbstractRenderer& renderer) const
{
  const auto transformation = this->transformation();
  {
    renderer.push_transformation(transformation);
    draw_arrow(renderer, X_STYLE);
    renderer.pop_transformation();
  }
  {
    renderer.push_transformation(transformation.rotated(M_PI/2.0));
    draw_arrow(renderer, Y_STYLE);
    renderer.pop_transformation();
  }
}

const std::set<Object*>& Handle::objects() const
{
  return m_objects;
}


ObjectTransformation GlobalOrientedHandle::transformation() const
{
  const auto add = [](const arma::vec2& accu, const Object* object) -> arma::vec2 {
    return accu + object->global_transformation().translation();
  };
  const auto sum_pos = std::accumulate(objects().begin(), objects().end(), arma::vec2(), add);
  return ObjectTransformation().translated(sum_pos / objects().size());
}

}  // namespace omm
