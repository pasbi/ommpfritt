#include "mainwindow/viewport/viewport.h"

#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "scene/scene.h"
#include "python/pythonengine.h"

namespace
{

void set_cursor_position(QWidget& widget, const omm::Vec2f& pos)
{
  auto cursor = widget.cursor();
  cursor.setPos(widget.mapToGlobal(pos.to_point()));
  widget.setCursor(cursor);
}

}  // namespace

namespace omm
{

Viewport::Viewport(Scene& scene)
  : m_scene(scene)
  , m_timer(std::make_unique<QTimer>())
  , m_pan_controller([this](const Vec2f& pos) { set_cursor_position(*this, pos); })
  , m_renderer(scene, AbstractRenderer::Category::All & ~AbstractRenderer::Category::BoundingBox)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy(Qt::StrongFocus);
  connect(&*m_timer, &QTimer::timeout, [this]() {
    update();
  });
  m_timer->setInterval(30);
  m_timer->start();

  setMouseTracking(true);
}

#if USE_OPENGL
void Viewport::paintGL()
#else
void Viewport::paintEvent(QPaintEvent*)
#endif
{
  QPainter painter(this);
  m_renderer.set_painter(painter);

  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  painter.fillRect(rect(), Qt::gray);

  m_scene.object_tree.root().set_transformation(viewport_transformation());
  m_scene.evaluate_tags();
  m_renderer.render();

  auto& tool = m_scene.tool_box.active_tool();
  tool.viewport_transformation = viewport_transformation();
  tool.draw(m_renderer);
  m_renderer.clear_painter();
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
  const Vec2f cursor_pos = Vec2f(event->pos());
  const auto action = [](const QMouseEvent* event) {
    switch (event->button()) {
    case Qt::LeftButton: return MousePanController::Action::Pan;
    case Qt::RightButton: return MousePanController::Action::Zoom;
    default: return MousePanController::Action::None;
    }
  };
  m_pan_controller.start_move( cursor_pos,
                               viewport_transformation().inverted().apply_to_position(cursor_pos),
                               action(event));

  if (event->modifiers() & Qt::AltModifier) {
    event->accept();
  } else {
    if (m_scene.tool_box.active_tool().mouse_press(cursor_pos, *event)) { event->accept(); }
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
  const auto cursor_pos = Vec2f(event->pos());

  Vec2f delta = Vec2f::o();
  if (event->modifiers() & Qt::AltModifier) {
    delta = m_pan_controller.apply(cursor_pos, m_viewport_transformation);
    event->accept();
  } else {
    delta = m_pan_controller.update(cursor_pos);
  }

  auto& tool = m_scene.tool_box.active_tool();
  if (tool.mouse_move(delta, cursor_pos, *event)) {
    event->accept();
    return;
  }

  ViewportBase::mouseMoveEvent(event);
}

void Viewport::mouseReleaseEvent(QMouseEvent* event)
{
  if (!m_pan_controller.end_move()) {
    const auto cursor_pos = Vec2f(event->pos());
    m_scene.tool_box.active_tool().mouse_release(cursor_pos, *event);
    if (event->button() == Qt::RightButton) {
      auto menu = m_scene.tool_box.active_tool().make_context_menu(this);
      if (menu) { menu->exec(event->globalPos()); }
    }
    ViewportBase::mouseReleaseEvent(event);
  }
}

ObjectTransformation Viewport::viewport_transformation() const
{
  return m_viewport_transformation.translated(Vec2f(width(), height())/2.0);
}

Scene& Viewport::scene() const { return m_scene; }
void Viewport::reset() { m_viewport_transformation = ObjectTransformation(); }

void Viewport::set_transformation(const ObjectTransformation& transformation)
{
  m_viewport_transformation = transformation;
  m_viewport_transformation.normalized();
}

void Viewport::keyPressEvent(QKeyEvent *event)
{
  if (!m_scene.tool_box.active_tool().key_press(*event)) {
    ViewportBase::keyPressEvent(event);
  }
}

}  // namespace omm
