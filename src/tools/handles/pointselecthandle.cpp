#include "tools/handles/pointselecthandle.h"

#include "commands/modifypointscommand.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "scene/scene.h"
#include "scene/mailbox.h"
#include "tools/tool.h"
#include "tools/selectpointsbasetool.h"

#include <QGuiApplication>
#include <QPainter>


namespace
{

auto make_tangent_handles_map(omm::Tool& tool, omm::PointSelectHandle& psh, const auto& tangent_keys)
{
  std::map<omm::Point::TangentKey, std::unique_ptr<omm::TangentHandle>> map;
  for (const auto& key : tangent_keys) {
    map.try_emplace(key, std::make_unique<omm::TangentHandle>(tool, psh, key));
  }
  return map;
}

}  // namespace

namespace omm
{

PointSelectHandle::PointSelectHandle(Tool& tool, PathObject& path_object, PathPoint& point)
    : AbstractSelectHandle(tool)
    , m_path_object(path_object)
    , m_point(point)
    , m_tangent_handles(make_tangent_handles_map(tool, *this, ::get_keys(point.geometry().tangents())))
{
  assert(m_point.path_vector());
  assert(&path_object == m_point.path_vector()->path_object());
  assert(path_object.scene() == tool.scene());
  assert(path_object.scene()->contains(&path_object));
}

ObjectTransformation PointSelectHandle::transformation() const
{
  return m_path_object.global_transformation(Space::Viewport);
}

bool PointSelectHandle::contains_global(const Vec2f& point) const
{
  const auto tpoint = transformation().apply_to_position(m_point.geometry().position());
  const auto d = (point - tpoint).euclidean_norm();
  return d < interact_epsilon();
}

bool PointSelectHandle::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  if (AbstractSelectHandle::mouse_press(pos, event)) {
    return true;
  }

  for (auto& [key, handle] : m_tangent_handles) {
    if (is_active(key) && handle->mouse_press(pos, event)) {
      return true;
    }
  }
  return false;
}

bool PointSelectHandle::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event)
{
  if (AbstractSelectHandle::mouse_move(delta, pos, event)) {
    return true;
  }

  for (auto& [key, handle] : m_tangent_handles) {
    if (is_active(key) && handle->mouse_move(delta, pos, event)) {
      return true;
    }
  }

  return false;
}

void PointSelectHandle::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  AbstractSelectHandle::mouse_release(pos, event);
  for (auto& [key, handle] : m_tangent_handles) {
    handle->mouse_release(pos, event);
  }
}

PathPoint& PointSelectHandle::point() const
{
  return m_point;
}

void PointSelectHandle::draw(QPainter& painter) const
{
  const auto pos = transformation().apply_to_position(m_point.geometry().position());
  for (const auto& [key, tangent] : m_point.geometry().tangents()) {
    if (is_active(key)) {
      const auto other_pos = transformation().apply_to_position(m_point.geometry().tangent_position(key));
      m_tangent_handles.at(key)->position = other_pos;
      painter.setPen(ui_color("tangent"));
      painter.drawLine(pos.x, pos.y, other_pos.x, other_pos.y);
      m_tangent_handles.at(key)->draw(painter);
    }
  }

  painter.translate(pos.to_pointf());
  const auto status = m_point.is_selected() ? HandleStatus::Active : this->status();
  const auto r = draw_epsilon();
  const auto rect = Tool::centered_rectangle({}, r);
  painter.setPen(ui_color(status, "point"));
  painter.setBrush(ui_color(status, "point fill"));
  painter.drawEllipse(rect);
}

void PointSelectHandle::transform_tangent(const Vec2f& delta, const Point::TangentKey& tangent_key)
{
  transform_tangent(delta, dynamic_cast<SelectPointsBaseTool&>(tool).tangent_mode(), tangent_key);
}

void PointSelectHandle::transform_tangent(const Vec2f& delta,
                                          TangentMode mode,
                                          const Point::TangentKey& primary_tangent_key)
{
  const auto old_primary_tangent = m_point.geometry().tangent(primary_tangent_key);
  auto new_point = m_point.geometry();

  auto& secondary_tangents = new_point.tangents();
  secondary_tangents.erase(primary_tangent_key);


  const auto t_delta = ObjectTransformation().translated(delta);
  const auto t_this = this->transformation();
  const auto new_primary_tangent = t_delta.transformed(t_this).apply_to_position(old_primary_tangent);

  std::map<PathPoint*, Point> map;
  if (mode == TangentMode::Mirror && !(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
    for (auto& [key, secondary] : secondary_tangents) {
      secondary = Point::mirror_tangent(secondary, old_primary_tangent, new_primary_tangent);
    }
  }

  new_point.set_tangent(primary_tangent_key, new_primary_tangent);

  map[&m_point] = new_point;
  tool.scene()->submit<ModifyPointsCommand>(map);
}

std::pair<bool, bool> PointSelectHandle::tangents_active() const
{
  const auto* interpolation_property = m_path_object.property(PathObject::INTERPOLATION_PROPERTY_KEY);
  const auto interpolation_mode = interpolation_property->value<InterpolationMode>();
  if ((interpolation_mode == InterpolationMode::Bezier && m_point.is_selected())) {
    const auto points = m_path_object.path_vector().find_path(m_point)->points();
    assert(!points.empty());
    return {points.front() != &m_point, points.back() != &m_point};
  } else {
    return {false, false};
  }
}

void PointSelectHandle::set_selected(bool selected)
{
  if (m_point.is_selected() != selected) {
    m_point.set_selected(selected);
    Q_EMIT tool.scene()->mail_box().point_selection_changed();
  }
}

void PointSelectHandle::clear()
{
  for (auto* point : m_path_object.path_vector().points()) {
    point->set_selected(false);
  }
}

bool PointSelectHandle::is_selected() const
{
  return m_point.is_selected();
}

std::map<Point::TangentKey, PolarCoordinates>
PointSelectHandle::other_tangents(const Point::TangentKey& tangent_key) const
{
  auto tangents = m_point.geometry().tangents();
  tangents.erase(tangent_key);
  return tangents;
}

bool PointSelectHandle::is_active(const Point::TangentKey& tangent_key) const
{
  if (tangent_key.path == nullptr) {
    LWARNING << "Unexpected condition: "
                "Tangent keys of points in PointSelectHandle should always have a path assigned.";
    return true;
  }

  if (tangent_key.direction == Direction::Backward) {
    // backward tangent is active if the point is not the first point in the path.
    return tangent_key.path->first_point().get() != &m_point;
  } else {
    // forward tangent is active if the point is not the last point in the path.
    return tangent_key.path->last_point().get() != &m_point;
  }
}

}  // namespace omm
