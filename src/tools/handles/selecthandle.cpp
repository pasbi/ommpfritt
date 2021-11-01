#include "tools/handles/selecthandle.h"
#include "commands/modifypointscommand.h"
#include "common.h"
#include "geometry/rectangle.h"
#include "geometry/vec2.h"
#include "objects/path.h"
#include "renderers/painter.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tools/selectpointstool.h"
#include "tools/selecttool.h"
#include "tools/tool.h"
#include <QGuiApplication>
#include <QMouseEvent>

namespace omm
{
AbstractSelectHandle::AbstractSelectHandle(Tool& tool)
  : Handle(tool)
{
}

bool AbstractSelectHandle::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  m_was_selected = is_selected();
  if (Handle::mouse_press(pos, event)) {
    QSignalBlocker blocker(tool.scene()->mail_box());
    if (!is_selected() && event.modifiers() != extend_selection_modifier) {
      clear();
    }
    set_selected(true);
    return true;
  } else {
    return false;
  }
}

void AbstractSelectHandle::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  if (status() == HandleStatus::Active) {
    static constexpr double eps = 0.0;
    if ((press_pos() - pos).euclidean_norm2() <= eps) {
      if (m_was_selected) {
        QSignalBlocker blocker(tool.scene()->mail_box());
        if (event.modifiers() == extend_selection_modifier) {
          set_selected(false);
        } else {
          clear();
          set_selected(true);
        }
      }
    }
  }
  Handle::mouse_release(pos, event);
}

bool AbstractSelectHandle::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  static constexpr double STEP_SIZE = 10.0;
  Handle::mouse_move(delta, pos, e);
  if (status() == HandleStatus::Active) {
    Vec2f total_delta = discretize(pos - press_pos(), false, STEP_SIZE);
    const auto transformation = omm::ObjectTransformation().translated(total_delta);
    total_delta = tool.viewport_transformation.inverted().apply_to_direction(total_delta);
    dynamic_cast<AbstractSelectTool&>(tool).transform_objects(transformation);
    const auto tool_info = QString("%1").arg(total_delta.euclidean_norm());
    dynamic_cast<AbstractSelectTool&>(tool).tool_info = tool_info;
    return true;
  } else {
    return false;
  }
}

ObjectSelectHandle::ObjectSelectHandle(Tool& tool, Scene& scene, Object& object)
    : AbstractSelectHandle(tool), m_scene(scene), m_object(object)
{
}

bool ObjectSelectHandle::contains_global(const Vec2f& point) const
{
  return (point - transformation().null()).max_norm() < interact_epsilon();
}

void ObjectSelectHandle::draw(QPainter& painter) const
{
  const auto is_selected = ::contains(m_scene.item_selection<Object>(), &m_object);

  const auto pos = transformation().null();
  const auto r = draw_epsilon();

  painter.setPen(is_selected ? ui_color(HandleStatus::Active, "object") : ui_color("object"));
  painter.setBrush(is_selected ? ui_color(HandleStatus::Active, "object fill")
                               : ui_color("object fill"));
  painter.drawRect(Tool::centered_rectangle(pos, r));
}

ObjectTransformation ObjectSelectHandle::transformation() const
{
  return m_object.global_transformation(Space::Viewport);
}

void ObjectSelectHandle::clear()
{
  m_scene.set_selection({});
}

void ObjectSelectHandle::set_selected(bool selected)
{
  auto selection = m_scene.item_selection<Object>();
  if (selected) {
    selection.insert(&m_object);
  } else {
    selection.erase(&m_object);
  }
  m_scene.set_selection(down_cast(selection));
}

bool ObjectSelectHandle::is_selected() const
{
  return ::contains(m_scene.item_selection<Object>(), &m_object);
}

PointSelectHandle::PointSelectHandle(Tool& tool, Path& path, Point& point)
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
  const auto tpoint = transformation().apply_to_position(m_point.position);
  const auto d = (point - tpoint).euclidean_norm();
  return d < interact_epsilon();
}

bool PointSelectHandle::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  if (AbstractSelectHandle::mouse_press(pos, event)) {
    return true;
  } else if (tangents_active()) {
    return m_left_tangent_handle->mouse_press(pos, event)
           || m_right_tangent_handle->mouse_press(pos, event);
  } else {
    return false;
  }
}

bool PointSelectHandle ::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event)
{
  if (AbstractSelectHandle::mouse_move(delta, pos, event)) {
    return true;
  } else if (tangents_active()) {
    return m_left_tangent_handle->mouse_move(delta, pos, event)
           || m_right_tangent_handle->mouse_move(delta, pos, event);
  } else {
    return false;
  }
}

void PointSelectHandle::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  AbstractSelectHandle::mouse_release(pos, event);
  m_left_tangent_handle->mouse_release(pos, event);
  m_right_tangent_handle->mouse_release(pos, event);
}

void PointSelectHandle::draw(QPainter& painter) const
{
  const auto pos = transformation().apply_to_position(m_point.position);
  const auto left_pos = transformation().apply_to_position(m_point.left_position());
  const auto right_pos = transformation().apply_to_position(m_point.right_position());

  const auto treat_sub_handle = [&painter, pos, this](auto& sub_handle, const auto& other_pos) {
    sub_handle.position = other_pos;

    painter.setPen(ui_color("tangent"));
    painter.drawLine(pos.x, pos.y, other_pos.x, other_pos.y);
    sub_handle.draw(painter);
  };

  if (tangents_active()) {
    treat_sub_handle(*m_right_tangent_handle, right_pos);
    treat_sub_handle(*m_left_tangent_handle, left_pos);
  }

  painter.translate(pos.to_pointf());
  const auto color = m_point.is_selected() ? ui_color(HandleStatus::Active, "point")
                                           : ui_color("point");

  const auto r = draw_epsilon();
  const QRectF rect{-r, -r, 2 * r, 2 * r};
  painter.setPen(color);
  painter.drawRect(rect);
  painter.fillRect(rect, color);
}

void PointSelectHandle::transform_tangent(const Vec2f& delta, TangentHandle::Tangent tangent)
{
  transform_tangent(delta, dynamic_cast<SelectPointsBaseTool&>(tool).tangent_mode(), tangent);
}

void PointSelectHandle::transform_tangent(const Vec2f& delta,
                                          TangentMode mode,
                                          TangentHandle::Tangent tangent)
{
  auto new_point = m_point;
  const auto [primary_pos, secondary_pos] = [tangent, &new_point]() {
    if (tangent == TangentHandle::Tangent::Left) {
      return std::pair{&new_point.left_tangent, &new_point.right_tangent};
    } else {
      return std::pair{&new_point.right_tangent, &new_point.left_tangent};
    }
  }();

  const auto old_primary_pos = *primary_pos;
  const auto transformation = ObjectTransformation().translated(delta);
  *primary_pos = transformation.transformed(this->transformation()).apply_to_position(*primary_pos);
  if (mode == TangentMode::Mirror
      && !(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
    *secondary_pos = Point::mirror_tangent(*secondary_pos, old_primary_pos, *primary_pos);
  }

  ModifyPointsCommand::Map map;
  map[&m_path][&m_point] = new_point;
  tool.scene()->submit<ModifyPointsCommand>(map);
}

bool PointSelectHandle::tangents_active() const
{
  const auto& imode_property = m_path.property(Path::INTERPOLATION_PROPERTY_KEY);
  const auto interpolation_mode = imode_property->value<InterpolationMode>();
  const bool is_bezier = interpolation_mode == InterpolationMode::Bezier;
  return (is_bezier && m_point.is_selected()) || force_draw_subhandles;
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
