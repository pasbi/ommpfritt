  #include "tools/handles/selecthandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"
#include "scene/scene.h"
#include "objects/path.h"
#include "common.h"
#include "tools/itemtools/selecttool.h"
#include "tools/itemtools/positionvariant.h"

namespace omm
{

ObjectSelectHandle
::ObjectSelectHandle(SelectTool<ObjectPositions>& tool, Scene& scene, Object& object)
  : m_tool(tool)
  , m_scene(scene)
  , m_object(object)
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

void ObjectSelectHandle
::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  Handle::mouse_move(delta, pos, allow_hover);
  if (status() == Status::Active) {
    const auto t = omm::ObjectTransformation().translated(delta);
    m_tool.transform_objects(t);
  }
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

PointSelectHandle::PointSelectHandle(SelectTool<PointPositions>& tool, Path& path, Point& point)
  : m_tool(tool)
  , m_path(path)
  , m_point(point)
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

ObjectTransformation PointSelectHandle::transformation() const
{
  return m_path.global_transformation();
}

bool PointSelectHandle::contains(const arma::vec2& point) const
{
  return arma::norm(point - m_point.position) < epsilon;
}

bool PointSelectHandle::mouse_press(const arma::vec2& pos)
{
  if (Handle::mouse_press(pos)) {
    m_point.is_selected = true; // !m_point.is_selected;
    return true;
  } else {
    return false;
  }
}

void PointSelectHandle
::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  Handle::mouse_move(delta, pos, allow_hover);
  if (status() == Status::Active) {
    const auto t = omm::ObjectTransformation().translated(delta);
    m_tool.transform_objects(t);
  }
}

void PointSelectHandle::draw(omm::AbstractRenderer& renderer) const
{
  const Style& style = m_point.is_selected ? this->style(Status::Active) : current_style();
  const auto pos = transformation().apply_to_position(m_point.position);
  renderer.draw_rectangle(pos, epsilon, style);
}

}  // namespace omm