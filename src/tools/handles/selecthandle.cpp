  #include "tools/handles/selecthandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"
#include "scene/scene.h"

namespace omm
{

ObjectSelectHandle::ObjectSelectHandle(Scene& scene, Object& object)
  : m_object(object)
  , m_scene(scene)
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

bool ObjectSelectHandle::contains(const arma::vec2& point) const
{
  return arma::norm(point) < epsilon;
}

void ObjectSelectHandle::draw(omm::AbstractRenderer& renderer) const
{
  const auto is_selected = [this]() {
    return ::contains(m_scene.object_selection(), &m_object);
  };

  const Style& style = is_selected() ? this->style(Status::Active) : current_style();
  const auto pos = transformation().apply_to_position(arma::vec2{ 0.0, 0.0 });
  renderer.draw_rectangle(pos, epsilon, style);
}

bool ObjectSelectHandle::mouse_press(const arma::vec2& pos)
{
  if (Handle::mouse_press(pos)) {
    auto selection = m_scene.object_selection();
    if (::contains(selection, &m_object)) {
      selection.erase(&m_object);
    } else {
      selection.insert(&m_object);
    }
    m_scene.set_selection(AbstractPropertyOwner::cast(selection));
    return true;
  } else {
    return false;
  }
}

ObjectTransformation ObjectSelectHandle::transformation() const
{
  return m_object.global_transformation();
}

PointSelectHandle::PointSelectHandle(Point& point) : m_point(point)
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

bool PointSelectHandle::contains(const arma::vec2& point) const
{
  return arma::norm(point) < epsilon;
}

void PointSelectHandle::draw(omm::AbstractRenderer& renderer) const
{
  renderer.draw_rectangle(arma::vec2{ 0.0, 0.0 }, epsilon, current_style());
}

}  // namespace omm