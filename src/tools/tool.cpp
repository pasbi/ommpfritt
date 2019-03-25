#include "tools/tool.h"

#include <memory>
#include "renderers/abstractrenderer.h"
#include "objects/object.h"
#include "properties/optionsproperty.h"
#include "scene/scene.h"
#include <QApplication>

namespace omm
{

Tool::Tool(Scene& scene)
  : scene(scene)
  , m_tool_info_line_style(ContourStyle(Color(0.0, 0.0, 0.0, 0.3), 0.7))
{ }

ObjectTransformation Tool::transformation() const { return ObjectTransformation(); }

bool Tool::mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e)
{
  m_current_position = transformation().null();
  for (auto&& handle : handles) {
    if (handle->is_enabled()) {
      handle->mouse_move(delta, pos, e);
      switch (handle->status()) {
      case Handle::Status::Active:
        return true;
      case Handle::Status::Hovered:
      case Handle::Status::Inactive:
        break;
      }
    }
  }
  return false;
}

bool Tool::mouse_press(const arma::vec2& pos, const QMouseEvent& e, bool force)
{
  // `std::any_of` does not *require* to use short-circuit-logic. However, here it is mandatory,
  // so don't use `std::any_of`.
  for (auto&& handle : handles) {
    if (handle->is_enabled() && handle->mouse_press(pos, e, force)) {
      reset_absolute_object_transformation();
      return true;
    }
  }
  reset_absolute_object_transformation();
  return false;
}

void Tool::
mouse_release(const arma::vec2& pos, const QMouseEvent& e)
{
  for (auto&& handle : handles) {
    if (handle->is_enabled()) {
      handle->mouse_release(pos, e);
    }
  }
}

void Tool::draw(AbstractRenderer& renderer) const
{
  renderer.set_category(AbstractRenderer::Category::Handles);
  const ObjectTransformation transformation = this->transformation();
  for (auto&& handle : handles) {
    if (handle->is_enabled()) {
      if (handle->transform_in_tool_space) {
        renderer.push_transformation(transformation);
        handle->draw(renderer);
        renderer.pop_transformation();
      } else {
        handle->draw(renderer);
      }
    }
  }
  if (!tool_info.empty()) {
    renderer.toast(m_current_position + arma::vec2{ 30.0, 30.0 }, tool_info.c_str());
    const auto line = std::vector { Point(m_init_position), Point(m_current_position) };
    renderer.draw_spline(line, m_tool_info_line_style, false);
  }
}

std::unique_ptr<QMenu> Tool::make_context_menu(QWidget* parent)
{
  Q_UNUSED(parent);
  return nullptr;
}

void Tool::transform_objects(ObjectTransformation t, const bool tool_space)
{
  Q_UNUSED(t);
  Q_UNUSED(tool_space);
}

bool Tool::has_transformation() const { return false; }
void Tool::on_selection_changed() {}
void Tool::on_scene_changed() {}
AbstractPropertyOwner::Flag Tool::flags() const { return Flag::None; }

void Tool::transform_objects_absolute(ObjectTransformation t, const bool tool_space)
{
  transform_objects(m_last_object_transformation.inverted().apply(t), tool_space);
  m_last_object_transformation = t;
}

void Tool::reset_absolute_object_transformation()
{
  LOG(INFO) << transformation().null();
  m_init_position = transformation().null();
  m_last_object_transformation = ObjectTransformation();
}

bool Tool::integer_transformation() const
{
  return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

}  // namespace omm
