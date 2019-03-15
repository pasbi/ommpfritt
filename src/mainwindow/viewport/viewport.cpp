#include "mainwindow/viewport/viewport.h"

#include <glog/logging.h>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "scene/scene.h"
#include "python/pythonengine.h"

namespace
{

arma::vec2 point2vec(const QPoint& p)
{
  return arma::vec2 {
    static_cast<double>(p.x()),
    static_cast<double>(p.y()),
  };
}

void set_cursor_position(QWidget& widget, const arma::vec2& pos)
{
  auto cursor = widget.cursor();
  cursor.setPos(widget.mapToGlobal(QPoint(pos(0), pos(1))));
  widget.setCursor(cursor);
}

}  // namespace

namespace omm
{

const ObjectTransformation Viewport::default_transformation
  = ObjectTransformation().scaled(arma::vec2{ 1.0, -1.0 });

Viewport::Viewport(Scene& scene)
  : m_scene(scene)
  , m_timer(std::make_unique<QTimer>())
  , m_viewport_transformation(default_transformation)
  , m_pan_controller([this](const arma::vec2& pos) { set_cursor_position(*this, pos); })
  , m_renderer(scene)
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
void Viewport::paintEvent(QPaintEvent* event)
#endif
{
  QPainter painter(this);
  m_renderer.set_painter(painter);

  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(rect(), Qt::gray);

  m_scene.object_tree.root().set_transformation(viewport_transformation());
  m_scene.evaluate_tags();
  m_renderer.render();

  m_scene.tool_box.active_tool().draw(m_renderer);
  m_renderer.clear_painter();
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
  const arma::vec2 cursor_pos = point2vec(event->pos());
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
  const auto cursor_pos = point2vec(event->pos());

  arma::vec2 delta { 0.0, 0.0 };
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

  QWidget::mouseMoveEvent(event);
}

void Viewport::mouseReleaseEvent(QMouseEvent* event)
{
  if (!m_pan_controller.end_move()) {
    const auto cursor_pos = point2vec(event->pos());
    m_scene.tool_box.active_tool().mouse_release(cursor_pos, *event);
    if (event->button() == Qt::RightButton) {
      auto menu = m_scene.tool_box.active_tool().make_context_menu(this);
      if (menu) { menu->exec(event->globalPos()); }
    }
    QWidget::mouseReleaseEvent(event);
  }
}

ObjectTransformation Viewport::viewport_transformation() const
{
  return m_viewport_transformation.translated(arma::vec2{ width() / 2.0, height() / 2.0 });
}

Scene& Viewport::scene() const { return m_scene; }
void Viewport::reset() { m_viewport_transformation = default_transformation; }

void Viewport::set_transformation(const ObjectTransformation& transformation)
{
  m_viewport_transformation = transformation;
  m_viewport_transformation.normalized();
}

BoundingBox Viewport::visible_rect(const arma::vec2 margin) const
{
  const arma::vec2 v { double(QWidget::width()), double(QWidget::height()) };
  BoundingBox bb({ margin, v - margin});
  return viewport_transformation().inverted().apply(bb);
}

}  // namespace omm
