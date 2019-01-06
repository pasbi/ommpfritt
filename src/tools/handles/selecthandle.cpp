  #include "tools/handles/selecthandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"
#include "scene/scene.h"
#include "objects/path.h"
#include "common.h"
#include "tools/itemtools/selecttool.h"
#include "tools/itemtools/positionvariant.h"

namespace
{

template<omm::PointSelectHandle::Tangent tangent>
class TangentHandle : public omm::ParticleHandle
{
public:
  TangentHandle(omm::PointSelectHandle& master_handle) : m_master_handle(master_handle) {}
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override
  {
    ParticleHandle::mouse_move(delta, pos, allow_hover);
    if (status() == Status::Active) {
      m_master_handle.transform_tangent<tangent>(delta);
      return true;
    } else {
      return false;
    }
  }

private:
  omm::PointSelectHandle& m_master_handle;
};

}  // namespace

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

bool ObjectSelectHandle
::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  Handle::mouse_move(delta, pos, allow_hover);
  if (status() == Status::Active) {
    const auto t = omm::ObjectTransformation().translated(delta);
    m_tool.transform_objects(t);
    return true;
  } else {
    return false;
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
  , m_tangent_style(std::make_unique<ContourStyle>(Color(0.0, 1.0, 0.0)))
  , m_left_tangent_handle(std::make_unique<TangentHandle<Tangent::Left>>(*this))
  , m_right_tangent_handle(std::make_unique<TangentHandle<Tangent::Right>>(*this))
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
  } else if (m_left_tangent_handle->mouse_press(pos)) {
    return true;
  } else if (m_right_tangent_handle->mouse_press(pos)) {
    return true;
  } else {
    return false;
  }
}

bool PointSelectHandle
::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover)
{
  Handle::mouse_move(delta, pos, allow_hover);
  if (status() == Status::Active) {
    m_tool.transform_objects(ObjectTransformation().translated(delta));
    return true;
  } else if (m_left_tangent_handle->mouse_move(delta, pos, allow_hover)) {
    return true;
  } else if (m_right_tangent_handle->mouse_move(delta, pos, allow_hover)) {
    return true;
  } else {
    return false;
  }
}

void PointSelectHandle::mouse_release()
{
  Handle::mouse_release();
  m_left_tangent_handle->deactivate();
  m_right_tangent_handle->deactivate();
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
    sub_handle.draw(renderer);
  };

  treat_sub_handle(*m_right_tangent_handle, right_pos);
  treat_sub_handle(*m_left_tangent_handle, left_pos);
  renderer.draw_rectangle(pos, epsilon, style);

}

template<PointSelectHandle::Tangent tangent>
void PointSelectHandle::transform_tangent(const arma::vec2& delta)
{
  transform_tangent<tangent>(delta, static_cast<SelectPointsTool&>(m_tool).tangent_mode());
}

template<PointSelectHandle::Tangent tangent>
void PointSelectHandle::transform_tangent(const arma::vec2& delta, TangentMode mode)
{
  auto& left_pos = m_point.left_tangent;
  auto& right_pos = m_point.right_tangent;
  auto& master_pos = ::conditional<tangent == Tangent::Left>(left_pos, right_pos);
  auto& slave_pos = ::conditional<tangent == Tangent::Left>(right_pos, left_pos);


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

}  // namespace omm