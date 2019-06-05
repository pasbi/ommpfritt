#include "tools/handles/selecthandle.h"
#include "geometry/vec2.h"
#include "renderers/painter.h"
#include "scene/scene.h"
#include "objects/path.h"
#include "common.h"
#include "tools/selecttool.h"
#include "commands/modifypointscommand.h"
#include <QGuiApplication>
#include <QMouseEvent>
#include "geometry/rectangle.h"
#include "tools/selectpointstool.h"

namespace
{

template<omm::PointSelectHandle::Tangent tangent>
class TangentHandle : public omm::ParticleHandle
{
public:
  TangentHandle(omm::Tool& tool, omm::PointSelectHandle& master_handle)
    : ParticleHandle(tool, false), m_master_handle(master_handle)
  {
    set_style(Handle::Status::Hovered, []() {
      omm::Style style(nullptr);
      style.property(omm::Style::COSMETIC_KEY)->set(true);
      style.property(omm::Style::BRUSH_COLOR_KEY)->set(omm::Color(1.0, 1.0, 0.0));
      style.property(omm::Style::PEN_COLOR_KEY)->set(omm::Color(0.0, 0.0, 1.0));
      style.property(omm::Style::BRUSH_IS_ACTIVE_KEY)->set(true);
      style.property(omm::Style::PEN_IS_ACTIVE_KEY)->set(true);
      return style;
    }());


    set_style(Handle::Status::Active, []() {
      omm::Style style(nullptr);
      style.property(omm::Style::COSMETIC_KEY)->set(true);
      style.property(omm::Style::BRUSH_COLOR_KEY)->set(omm::Color(1.0, 1.0, 1.0));
      style.property(omm::Style::PEN_COLOR_KEY)->set(omm::Color(0.0, 0.0, 0.0));
      style.property(omm::Style::BRUSH_IS_ACTIVE_KEY)->set(true);
      style.property(omm::Style::PEN_IS_ACTIVE_KEY)->set(true);
      return style;
    }());

    set_style(Handle::Status::Inactive, []() {
      omm::Style style(nullptr);
      style.property(omm::Style::COSMETIC_KEY)->set(true);
      style.property(omm::Style::BRUSH_COLOR_KEY)->set(omm::Color(0.8, 0.8, 0.2));
      style.property(omm::Style::PEN_COLOR_KEY)->set(omm::Color(0.2, 0.2, 0.8));
      style.property(omm::Style::BRUSH_IS_ACTIVE_KEY)->set(true);
      style.property(omm::Style::PEN_IS_ACTIVE_KEY)->set(true);
      return style;
    }());
  }

  bool mouse_move(const omm::Vec2f& delta, const omm::Vec2f& pos, const QMouseEvent& e) override
  {
    ParticleHandle::mouse_move(delta, pos, e);
    if (status() == Status::Active) {
      m_master_handle.transform_tangent<tangent>(delta);
      return true;
    } else {
      return false;
    }
  }

  double draw_epsilon() const override { return 2.0; }

  void draw(omm::Painter& renderer) const override
  {
    renderer.set_style(current_style());
    const auto r = draw_epsilon();
    renderer.painter->drawEllipse(position.x - r, position.y - r, 2*r, 2*r);
  }

private:
  omm::PointSelectHandle& m_master_handle;
};

}  // namespace

namespace omm
{

AbstractSelectHandle::AbstractSelectHandle(Tool& tool) : Handle(tool, false) { }

bool AbstractSelectHandle::mouse_press(const Vec2f &pos, const QMouseEvent &event, bool force)
{
  if (Handle::mouse_press(pos, event, force)) {
    if (force) {
      if (event.modifiers() != extend_selection_modifier) {
        clear();
      }
      set_selected(true);
      return true;
    } else {
      return is_selected();
    }
  } else {
    return false;
  }
}

bool AbstractSelectHandle::
mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  Handle::mouse_move(delta, pos, e);
  if (status() == Status::Active) {
    Vec2f total_delta = pos - press_pos();
    discretize(total_delta);
    const auto transformation = omm::ObjectTransformation().translated(total_delta);
    static_cast<AbstractSelectTool&>(tool).transform_objects_absolute(transformation, false);
    const auto tool_info = QString("%1").arg(total_delta.euclidean_norm());
    static_cast<AbstractSelectTool&>(tool).tool_info = tool_info.toStdString();
    return true;
  } else {
    return false;
  }
}

ObjectSelectHandle::ObjectSelectHandle(Tool& tool, Scene& scene, Object& object)
  : AbstractSelectHandle(tool)
  , m_scene(scene)
  , m_object(object)
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

bool ObjectSelectHandle::contains_global(const Vec2f& point) const
{
  return (point - transformation().null()).max_norm() < interact_epsilon();
}

void ObjectSelectHandle::draw(Painter &renderer) const
{
  const auto is_selected = [this]() {
    return ::contains(m_scene.item_selection<Object>(), &m_object);
  };

  const Style& style = is_selected() ? this->style(Status::Active) : current_style();
  const auto pos = transformation().null();
  const auto r = draw_epsilon();

  renderer.set_style(style);
  renderer.painter->drawRect(pos.x - r, pos.y - r, 2*r, 2*r);
}


ObjectTransformation ObjectSelectHandle::transformation() const
{
  return m_object.global_transformation();
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
  , m_tangent_style(std::make_unique<ContourStyle>(Color(0.0, 0.0, 0.0), 0.1))
  , m_left_tangent_handle(std::make_unique<TangentHandle<Tangent::Left>>(tool, *this))
  , m_right_tangent_handle(std::make_unique<TangentHandle<Tangent::Right>>(tool, *this))
{
  set_style(Status::Hovered, omm::SolidStyle(omm::Color(1.0, 1.0, 0.0)));
  set_style(Status::Active, omm::SolidStyle(omm::Color(1.0, 1.0, 1.0)));
  set_style(Status::Inactive, omm::SolidStyle(omm::Color(0.8, 0.8, 0.2)));
}

ObjectTransformation PointSelectHandle::transformation() const
{
  return m_path.global_transformation();
}

bool PointSelectHandle::contains_global(const Vec2f& point) const
{
  const auto d = (point - transformation().apply_to_position(m_point.position)).euclidean_norm();
  return d < interact_epsilon();
}

bool PointSelectHandle::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force)
{
  if (AbstractSelectHandle::mouse_press(pos, event, force)) {
    return true;
  } else if (tangents_active() && force && m_left_tangent_handle->mouse_press(pos, event, false)) {
    return true;
  } else if (tangents_active() && force && m_right_tangent_handle->mouse_press(pos, event, false)) {
    return true;
  } else {
    return false;
  }
}

bool PointSelectHandle
::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event)
{
  if (AbstractSelectHandle::mouse_move(delta, pos, event)) {
    return true;
  } else if (tangents_active() && m_left_tangent_handle->mouse_move(delta, pos, event)) {
    return true;
  } else if (tangents_active() && m_right_tangent_handle->mouse_move(delta, pos, event)) {
    return true;
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

void PointSelectHandle::draw(Painter &renderer) const
{
  const Style& style = m_point.is_selected ? this->style(Status::Active) : current_style();
  const auto pos = transformation().apply_to_position(m_point.position);
  const auto left_pos = transformation().apply_to_position(m_point.left_position());
  const auto right_pos = transformation().apply_to_position(m_point.right_position());

  const auto treat_sub_handle = [&renderer, pos, this](auto& sub_handle, const auto& other_pos) {
    sub_handle.position = other_pos;

    renderer.set_style(*m_tangent_style);
    renderer.painter->drawLine(pos.x, pos.y, other_pos.x, other_pos.y);
    sub_handle.draw(renderer);
  };

  if (tangents_active()) {
    treat_sub_handle(*m_right_tangent_handle, right_pos);
    treat_sub_handle(*m_left_tangent_handle, left_pos);
  }

  renderer.push_transformation(ObjectTransformation().translated(pos));
  renderer.set_style(style);

  const auto r = draw_epsilon();
  renderer.painter->drawRect(-r, -r, 2*r, 2*r);
  renderer.pop_transformation();

}

template<PointSelectHandle::Tangent tangent>
void PointSelectHandle::transform_tangent(const Vec2f& delta)
{
  transform_tangent<tangent>(delta, static_cast<SelectPointsTool&>(tool).tangent_mode());
}

template<PointSelectHandle::Tangent tangent>
void PointSelectHandle::transform_tangent(const Vec2f& delta, TangentMode mode)
{
  auto new_point = m_point;
  {
    auto& master_pos = ::conditional<tangent == Tangent::Left>( new_point.left_tangent,
                                                                new_point.right_tangent );
    auto& slave_pos  = ::conditional<tangent == Tangent::Left>( new_point.right_tangent,
                                                                new_point.left_tangent  );

    const auto old_master_pos = master_pos;
    const auto transformation = ObjectTransformation().translated(delta);
    master_pos = transformation.transformed(this->transformation()).apply_to_position(master_pos);
    if (mode == TangentMode::Mirror && !(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier))
    {
      slave_pos = Point::mirror_tangent(slave_pos, old_master_pos, master_pos);
    }
  }

  std::map<Path*, std::map<Point*, Point>> map;
  map[&m_path][&m_point] = new_point;
  tool.scene.submit<ModifyPointsCommand>(map);
}

bool PointSelectHandle::tangents_active() const
{
  const auto& imode_property = m_path.property(Path::INTERPOLATION_PROPERTY_KEY);
  const auto interpolation_mode = imode_property->value<Path::InterpolationMode>();
  const bool is_bezier = interpolation_mode == Path::InterpolationMode::Bezier;
  return (is_bezier && m_point.is_selected) || force_draw_subhandles;
}

void PointSelectHandle::set_selected(bool selected)
{
  m_point.is_selected = selected;
}

void PointSelectHandle::clear()
{
  m_path.deselect_all_points();
}

bool PointSelectHandle::is_selected() const
{
  return m_point.is_selected;
}

}  // namespace omm
