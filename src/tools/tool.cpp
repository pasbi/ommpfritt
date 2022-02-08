#include "tools/tool.h"

#include "objects/object.h"
#include "properties/optionproperty.h"
#include "renderers/painter.h"
#include "scene/scene.h"
#include <QApplication>
#include <QKeyEvent>
#include <memory>

namespace omm
{
Tool::Tool(Scene& scene) : PropertyOwner(&scene)
{
}

ObjectTransformation Tool::transformation() const
{
  return ObjectTransformation();
}

Flag Tool::flags() const
{
  return Flag::None;
}

bool Tool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  for (auto it = m_handles.rbegin(); it != m_handles.rend(); ++it) {
    auto& handle = **it;
    handle.mouse_move(delta, pos, e);
    switch (handle.status()) {
    case HandleStatus::Active:
      return true;
    case HandleStatus::Hovered:
    case HandleStatus::Inactive:
      break;
    }
  }
  return false;
}

bool Tool::mouse_press(const Vec2f& pos, const QMouseEvent& e)
{
  // `std::any_of` does not *require* to use short-circuit-logic. However, here it is mandatory,
  // so don't use `std::any_of`.
  for (auto it = m_handles.rbegin(); it != m_handles.rend(); ++it) {
    if ((*it)->mouse_press(pos, e)) {
      return true;
    }
  }
  return false;
}

void Tool::mouse_release(const Vec2f& pos, const QMouseEvent& e)
{
  for (auto it = m_handles.rbegin(); it != m_handles.rend(); ++it) {
    (*it)->mouse_release(pos, e);
  }
}

void Tool::draw(Painter& renderer) const
{
  if (!!(renderer.category_filter & Painter::Category::Handles)) {
    QPainter& painter = *renderer.painter;
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    for (const auto* handle : handles()) {
      painter.save();
      painter.setBrush(Qt::NoBrush);
      painter.setPen(Qt::NoPen);
      handle->draw(painter);
      painter.restore();
    }
    painter.restore();
  }
}


bool Tool::is_active() const
{
  for (const auto* handle : handles()) {
    std::cout << "handle " << handle << ": " << int(handle->status()) << std::endl;
  }
  return std::any_of(m_handles.begin(), m_handles.end(), [](const auto& handle) {
    return handle->status() == HandleStatus::Active;
  });
}

std::unique_ptr<QMenu> Tool::make_context_menu(QWidget* parent)
{
  Q_UNUSED(parent);
  return nullptr;
}

bool Tool::integer_transformation()
{
  return (QApplication::keyboardModifiers() & Qt::ShiftModifier) != 0u;
}

bool Tool::key_press(const QKeyEvent& event)
{
  return event.key() == Qt::Key_Escape && cancel();
}

bool Tool::cancel()
{
  for (auto* handle : handles()) {
    handle->deactivate();
  }
  return false;
}

void Tool::end()
{
}

void Tool::start()
{
}

QString Tool::name() const
{
  return QCoreApplication::translate("any-context", type().toUtf8().constData());
}

QRectF Tool::centered_rectangle(const Vec2f& center, double radius)
{
  return QRectF{QPointF{center.x - radius, center.y - radius}, 2.0 * QSizeF{radius, radius}};
}

void Tool::reset()
{
}

void Tool::clear()
{
  m_handles.clear();
}

std::deque<Handle*> Tool::handles() const
{
  return util::transform(m_handles, [](const auto& handle) { return handle.get(); });
}

void Tool::push_handle(std::unique_ptr<Handle> handle)
{
  m_handles.push_back(std::move(handle));
}

}  // namespace omm
