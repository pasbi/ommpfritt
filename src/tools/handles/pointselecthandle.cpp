#include "tools/handles/pointselecthandle.h"

#include "commands/modifypointscommand.h"
#include "objects/path.h"
#include "objects/pathpoint.h"
#include "objects/segment.h"
#include "scene/scene.h"
#include "scene/mailbox.h"
#include "tools/tool.h"
#include "tools/selectpointsbasetool.h"

#include <QGuiApplication>
#include <QPainter>

namespace omm
{

PointSelectHandle::PointSelectHandle(Tool& tool, Path& path, PathPoint& point)
    : AbstractSelectHandle(tool)
    , m_path(path)
    , m_point(point)
    , m_left_tangent_handle(
          std::make_unique<TangentHandle>(tool, *this, TangentHandle::Tangent::Left))
    , m_right_tangent_handle(
          std::make_unique<TangentHandle>(tool, *this, TangentHandle::Tangent::Right))
{
}

ObjectTransformation PointSelectHandle::transformation() const
{
  return m_path.global_transformation(Space::Viewport);
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

void PointSelectHandle::transform_tangent(const Vec2f& delta,
                                          TangentMode mode,
                                          TangentHandle::Tangent tangent)
{
  auto new_point = m_point.geometry();
  PolarCoordinates primary_pos;
  PolarCoordinates secondary_pos;
  if (tangent == TangentHandle::Tangent::Right) {
    primary_pos = new_point.right_tangent();
    secondary_pos = new_point.left_tangent();
  } else {
    primary_pos = new_point.left_tangent();
    secondary_pos = new_point.right_tangent();
  }

  const auto old_primary_pos = primary_pos;
  const auto transformation = ObjectTransformation().translated(delta);
  primary_pos = transformation.transformed(this->transformation()).apply_to_position(primary_pos);
  if (mode == TangentMode::Mirror && !(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
    secondary_pos = Point::mirror_tangent(secondary_pos, old_primary_pos, primary_pos);
  }

  if (tangent == TangentHandle::Tangent::Right) {
    new_point.set_right_tangent(primary_pos);
    new_point.set_left_tangent(secondary_pos);
  } else {
    new_point.set_left_tangent(primary_pos);
    new_point.set_right_tangent(secondary_pos);
  }

  std::map<PathPoint*, Point> map;
  map[&m_point] = new_point;
  tool.scene()->submit<ModifyPointsCommand>(map);
}

std::pair<bool, bool> PointSelectHandle::tangents_active() const
{
  const auto interpolation_mode = m_path.property(Path::INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();
  if ((interpolation_mode == InterpolationMode::Bezier && m_point.is_selected()) || force_draw_subhandles) {
    const auto points = m_path.find_segment(m_point)->points();
    assert(!points.empty());
    return {m_path.is_closed() || points.front() != &m_point,
            m_path.is_closed() || points.back() != &m_point};
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
  for (auto* point : m_path.points()) {
    point->set_selected(false);
  }
}

bool PointSelectHandle::is_selected() const
{
  return m_point.is_selected();
}

}  // namespace omm
