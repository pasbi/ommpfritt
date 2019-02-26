  #include "tools/handles/selecthandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"
#include "scene/scene.h"
#include "objects/path.h"
#include "common.h"
#include "tools/selecttool.h"
#include "tools/positionvariant.h"
#include "commands/modifypointscommand.h"
#include <QMouseEvent>

namespace
{

template<omm::PointSelectHandle::Tangent tangent>
class TangentHandle : public omm::ParticleHandle
{
public:
  TangentHandle(omm::Tool& tool, omm::PointSelectHandle& master_handle)
    : ParticleHandle(tool, false), m_master_handle(master_handle)
  {
    set_style(Handle::Status::Inactive, omm::SolidStyle(omm::Color(0.0, 0.0, 0.0)));
  }

  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override
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

private:
  omm::PointSelectHandle& m_master_handle;
};

}  // namespace

namespace omm
{

AbstractSelectHandle::AbstractSelectHandle(Tool& tool) : Handle(tool, false) { }

bool AbstractSelectHandle::mouse_press(const arma::vec2 &pos, const QMouseEvent &event)
{
  m_move_was_performed = false;
  return false;
}

bool AbstractSelectHandle
::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& event)
{
  m_move_was_performed = true;
  return Handle::mouse_move(delta, pos, event);
}

void AbstractSelectHandle::mouse_release(const arma::vec2& pos, const QMouseEvent& event)
{
  if (contains_global(pos)) {
    if (!m_move_was_performed) {
      if (event.button() == Qt::LeftButton) {
        if (event.modifiers() == extend_selection_modifier) {
          set_selected(true);
        } else {
          clear();
          set_selected(true);
        }
      } else if (event.button() == Qt::RightButton) {
        if (!is_selected()) {
          clear();
          set_selected(true);
        }
      }
    }
  }
  Handle::mouse_release(pos, event);
  m_move_was_performed = false;
}

void AbstractSelectHandle::report_move_action()
{
  m_move_was_performed = true;
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

bool ObjectSelectHandle::contains_global(const arma::vec2& point) const
{
  const auto d = arma::norm(point - transformation().apply_to_position(arma::vec2{0.0, 0.0}));
  return d < interact_epsilon();
}

void ObjectSelectHandle::draw(omm::AbstractRenderer& renderer) const
{
  const auto is_selected = [this]() {
    return ::contains(m_scene.item_selection<Object>(), &m_object);
  };

  const Style& style = is_selected() ? this->style(Status::Active) : current_style();
  const auto pos = transformation().apply_to_position(arma::vec2{ 0.0, 0.0 });
  renderer.draw_rectangle(pos, draw_epsilon(), style);
}

bool ObjectSelectHandle
::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& event)
{
  AbstractSelectHandle::mouse_move(delta, pos, event);
  if (status() == Status::Active) {
    const auto t = omm::ObjectTransformation().translated(delta);
    tool.transform_objects(t, false);
    return true;
  } else {
    return false;
  }
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
  m_scene.set_selection(AbstractPropertyOwner::cast(selection));
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

bool PointSelectHandle::contains_global(const arma::vec2& point) const
{
  const auto d = arma::norm(point - transformation().apply_to_position(m_point.position));
  return d < interact_epsilon();
}

bool PointSelectHandle::mouse_press(const arma::vec2& pos, const QMouseEvent& event)
{
  if (AbstractSelectHandle::mouse_press(pos, event)) {
    return true;
  } else if (tangents_active() && m_left_tangent_handle->mouse_press(pos, event)) {
    return true;
  } else if (tangents_active() && m_right_tangent_handle->mouse_press(pos, event)) {
    return true;
  } else {
    return false;
  }
}

bool PointSelectHandle
::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& event)
{
  AbstractSelectHandle::mouse_move(delta, pos, event);
  if (status() == Status::Active) {
    tool.transform_objects(ObjectTransformation().translated(delta), false);
    return true;
  } else if (tangents_active() && m_left_tangent_handle->mouse_move(delta, pos, event)) {
    return true;
  } else if (tangents_active() && m_right_tangent_handle->mouse_move(delta, pos, event)) {
    return true;
  } else {
    return false;
  }
}

void PointSelectHandle::mouse_release(const arma::vec2& pos, const QMouseEvent& event)
{
  AbstractSelectHandle::mouse_release(pos, event);
  m_left_tangent_handle->mouse_release(pos, event);
  m_right_tangent_handle->mouse_release(pos, event);
}

void PointSelectHandle::draw(omm::AbstractRenderer& renderer) const
{
  const Style& style = m_point.is_selected ? this->style(Status::Active) : current_style();
  const auto pos = transformation().apply_to_position(m_point.position);
  const auto left_pos = transformation().apply_to_position(m_point.left_position());
  const auto right_pos = transformation().apply_to_position(m_point.right_position());

  const auto treat_sub_handle = [&renderer, pos, this](auto& sub_handle, const auto& other_pos) {
    sub_handle.position = other_pos;
    renderer.draw_spline( { Point(pos), Point(other_pos) }, *m_tangent_style);
    if (m_point.is_selected) { sub_handle.draw(renderer); }
  };

  if (tangents_active()) {
    treat_sub_handle(*m_right_tangent_handle, right_pos);
    treat_sub_handle(*m_left_tangent_handle, left_pos);
  }
  renderer.draw_rectangle(pos, draw_epsilon(), style);

}

template<PointSelectHandle::Tangent tangent>
void PointSelectHandle::transform_tangent(const arma::vec2& delta)
{
  transform_tangent<tangent>(delta, static_cast<SelectPointsTool&>(tool).tangent_mode());
}

template<PointSelectHandle::Tangent tangent>
void PointSelectHandle::transform_tangent(const arma::vec2& delta, TangentMode mode)
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
    if (mode == TangentMode::Mirror) {
      static constexpr double mag_eps = 0.00001;
      slave_pos.argument += master_pos.argument - old_master_pos.argument;
      if (old_master_pos.magnitude > mag_eps) {
        slave_pos.magnitude *= master_pos.magnitude / old_master_pos.magnitude;
      }
    }
  }

  std::map<Path*, std::map<Point*, Point>> map;
  map[&m_path][&m_point] = new_point;
  tool.scene.submit<ModifyPointsCommand>(map);
}

bool PointSelectHandle::tangents_active() const
{
  const auto& imode_property = m_path.property(Path::INTERPOLATION_PROPERTY_KEY);
  const auto interpolation_mode = imode_property.value<Path::InterpolationMode>();
  const bool is_bezier = interpolation_mode == Path::InterpolationMode::Bezier;
  return is_bezier && m_point.is_selected;
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
