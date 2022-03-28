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

namespace omm
{

PointSelectHandle::PointSelectHandle(Tool& tool, PathObject& path_object, PathPoint& point)
    : AbstractSelectHandle(tool)
    , m_path_object(path_object)
    , m_point(point)
    , m_left_tangent_handle(std::make_unique<TangentHandle>(tool, *this, TangentHandle::Tangent::Left))
    , m_right_tangent_handle(std::make_unique<TangentHandle>(tool, *this, TangentHandle::Tangent::Right))
{
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

  const auto [left_tangent_active, right_tangent_active] = tangents_active();
  if (left_tangent_active && m_left_tangent_handle->mouse_press(pos, event)) {
    return true;
  }
  if (right_tangent_active && m_right_tangent_handle->mouse_press(pos, event)) {
    return true;
  }
  return false;
}

bool PointSelectHandle ::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event)
{
  if (AbstractSelectHandle::mouse_move(delta, pos, event)) {
    return true;
  }

  const auto [left_tangent_active, right_tangent_active] = tangents_active();
  if (left_tangent_active && m_left_tangent_handle->mouse_move(delta, pos, event)) {
    return true;
  }
  if (right_tangent_active && m_right_tangent_handle->mouse_move(delta, pos, event)) {
    return true;
  }

  return false;
}

void PointSelectHandle::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  AbstractSelectHandle::mouse_release(pos, event);
  m_left_tangent_handle->mouse_release(pos, event);
  m_right_tangent_handle->mouse_release(pos, event);
}

PathPoint& PointSelectHandle::point() const
{
  return m_point;
}

void PointSelectHandle::draw(QPainter& painter) const
{
  const auto pos = transformation().apply_to_position(m_point.geometry().position());

  const auto treat_sub_handle = [&painter, pos, this](auto& sub_handle, const auto& other_pos) {
    sub_handle.position = other_pos;

    painter.setPen(ui_color("tangent"));
    painter.drawLine(pos.x, pos.y, other_pos.x, other_pos.y);
    sub_handle.draw(painter);
  };

  const auto [left_tangent_active, right_tangent_active] = tangents_active();
  if (left_tangent_active) {
    const auto left_pos = transformation().apply_to_position(m_point.geometry().left_position());
    treat_sub_handle(*m_left_tangent_handle, left_pos);
  }
  if (right_tangent_active) {
    const auto right_pos = transformation().apply_to_position(m_point.geometry().right_position());
    treat_sub_handle(*m_right_tangent_handle, right_pos);
  }

  painter.translate(pos.to_pointf());
  const auto status = m_point.is_selected() ? HandleStatus::Active : this->status();
  const auto r = draw_epsilon();
  const auto rect = Tool::centered_rectangle({}, r);
  painter.setPen(ui_color(status, "point"));
  painter.setBrush(ui_color(status, "point fill"));
  if (m_point.joined_points().size() > 1) {
    painter.drawRect(rect);
  } else {
    painter.drawEllipse(rect);
  }
}

void PointSelectHandle::transform_tangent(const Vec2f& delta, TangentHandle::Tangent tangent)
{
  transform_tangent(delta, dynamic_cast<SelectPointsBaseTool&>(tool).tangent_mode(), tangent);
}

auto get_primary_secondary_tangent(const Point& point, const TangentHandle::Tangent tangent)
{
  switch (tangent) {
  case TangentHandle::Tangent::Right:
    return std::pair{point.right_tangent(), point.left_tangent()};
  case TangentHandle::Tangent::Left:
    return std::pair{point.left_tangent(), point.right_tangent()};
  }
  Q_UNREACHABLE();
}

void set_primary_secondary_tangent(Point& point,
                                   const PolarCoordinates& primary,
                                   const PolarCoordinates& secondary,
                                   const TangentHandle::Tangent tangent)
{
  switch (tangent) {
  case TangentHandle::Tangent::Left:
    point.set_left_tangent(primary);
    point.set_right_tangent(secondary);
    return;
  case TangentHandle::Tangent::Right:
    point.set_left_tangent(secondary);
    point.set_right_tangent(primary);
    return;
  }
  Q_UNREACHABLE();
}

void PointSelectHandle::transform_tangent(const Vec2f& delta,
                                          TangentMode mode,
                                          TangentHandle::Tangent tangent)
{
  auto new_point = m_point.geometry();
  const auto [primary, secondary] = get_primary_secondary_tangent(new_point, tangent);

  const auto transformation = ObjectTransformation().translated(delta);
  const auto new_primary = transformation.transformed(this->transformation()).apply_to_position(primary);
  auto new_secondary = secondary;

  std::map<PathPoint*, Point> map;
  if (mode == TangentMode::Mirror && !(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
    new_secondary = Point::mirror_tangent(secondary, primary, new_primary);
    for (auto* buddy : m_point.joined_points()) {
      if (buddy != &m_point) {
        auto geometry = buddy->geometry();
        const auto left = Point::mirror_tangent(geometry.left_tangent(), primary, new_primary);
        const auto right = Point::mirror_tangent(geometry.right_tangent(), primary, new_primary);
        geometry.set_left_tangent(left);
        geometry.set_right_tangent(right);
        map[buddy] = geometry;
      }
    }
  }

  set_primary_secondary_tangent(new_point, new_primary, new_secondary, tangent);

  map[&m_point] = new_point;
  tool.scene()->submit<ModifyPointsCommand>(map);
}

std::pair<bool, bool> PointSelectHandle::tangents_active() const
{
  const auto* interpolation_property = m_path_object.property(PathObject::INTERPOLATION_PROPERTY_KEY);
  const auto interpolation_mode = interpolation_property->value<InterpolationMode>();
  if ((interpolation_mode == InterpolationMode::Bezier && m_point.is_selected())) {
    const auto points = m_path_object.geometry().find_path(m_point)->points();
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
  for (auto* point : m_path_object.geometry().points()) {
    point->set_selected(false);
  }
}

bool PointSelectHandle::is_selected() const
{
  return m_point.is_selected();
}

}  // namespace omm
