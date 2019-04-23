#include "tools/tool.h"

#include <memory>
#include "renderers/abstractrenderer.h"
#include "objects/object.h"
#include "properties/optionsproperty.h"
#include "scene/scene.h"
#include <QApplication>
#include <QKeyEvent>

namespace omm
{

Tool::Tool(Scene& scene)
  : scene(scene)
{ }

ObjectTransformation Tool::transformation() const { return ObjectTransformation(); }

bool Tool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  for (auto&& handle : handles) {
    assert(handle != nullptr);
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

bool Tool::mouse_press(const Vec2f& pos, const QMouseEvent& e, bool force)
{
  // `std::any_of` does not *require* to use short-circuit-logic. However, here it is mandatory,
  // so don't use `std::any_of`.
  for (auto&& handle : handles) {
    if (handle->is_enabled() && handle->mouse_press(pos, e, force)) {
      return true;
    }
  }
  return false;
}

void Tool::
mouse_release(const Vec2f& pos, const QMouseEvent& e)
{
  for (auto&& handle : handles) {
    if (handle->is_enabled()) {
      handle->mouse_release(pos, e);
    }
  }
}

void Tool::draw(AbstractRenderer& renderer) const
{
  if (!!(renderer.category_filter & AbstractRenderer::Category::Handles)) {
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
  }
}

std::unique_ptr<QMenu> Tool::make_context_menu(QWidget* parent)
{
  Q_UNUSED(parent);
  return nullptr;
}

bool Tool::has_transformation() const { return false; }
void Tool::on_selection_changed() {}
void Tool::on_scene_changed() {}
AbstractPropertyOwner::Flag Tool::flags() const { return Flag::None; }

bool Tool::integer_transformation() const
{
  return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

bool Tool::key_press(const QKeyEvent &event)
{
  if (event.key() == Qt::Key_Escape) {
    cancel();
    return true;
  } else {
    return false;
  }
}

void Tool::cancel()
{
  for (auto&& handle : handles) {
    handle->deactivate();
  }
}

std::string Tool::name() const
{
  return QCoreApplication::translate("any-context", type().c_str()).toStdString();
}

}  // namespace omm
