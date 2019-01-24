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

// coordinate system of QWidget's canvas goes top-down and left-to-right
// I think bottom-up and left-to-right is more intuitive.
const auto TOP_RIGHT = omm::ObjectTransformation().scaled({1, -1});

}  // namespace

namespace omm
{

Viewport::Viewport(Scene& scene)
  : m_scene(scene)
  , m_timer(std::make_unique<QTimer>())
  , m_pan_controller([this](const arma::vec2& pos) { set_cursor_position(*this, pos); })
  , m_viewport_transformation(TOP_RIGHT)
  , m_renderer(scene)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
  m_renderer.set_base_transformation(viewport_transformation());

  m_scene.evaluate_tags();
  m_renderer.render();

  m_scene.tool_box.active_tool().draw(m_renderer);
  m_renderer.clear_painter();
}

arma::vec2 Viewport::viewport_to_global_direction(const arma::vec2& pos) const
{
  return viewport_transformation().inverted().apply_to_direction(pos);
}

arma::vec2 Viewport::viewport_to_global_position(const arma::vec2& pos) const
{
  return viewport_transformation().inverted().apply_to_position(pos);
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
  const arma::vec2 cursor_position = point2vec(event->pos());
  m_pan_controller.init(cursor_position);

  if (event->modifiers() & Qt::AltModifier) {
    event->accept();
  } else if ( const auto pos = viewport_to_global_position(cursor_position);
              m_scene.tool_box.active_tool().mouse_press(pos, *event)       )
  {
    event->accept();
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
  const auto widget_size = arma::vec2{ static_cast<double>(width()),
                                       static_cast<double>(height()) };
  const auto cursor_position = point2vec(event->pos());
  arma::vec2 delta { 0.0, 0.0 };
  if (event->buttons() == Qt::LeftButton)
  {
    delta = m_pan_controller.delta(cursor_position, widget_size);
    if (event->modifiers() & Qt::AltModifier) {
      m_viewport_transformation.translate(delta);
      event->accept();
      return;
    }
  }

  auto& tool = m_scene.tool_box.active_tool();
  const auto delta_ = viewport_to_global_direction(delta);
  const auto cpos_ = viewport_to_global_position(cursor_position);
  if (tool.mouse_move(delta_, cpos_, *event)) {
    event->accept();
    return;
  }

  QWidget::mouseMoveEvent(event);
}

void Viewport::mouseReleaseEvent(QMouseEvent* event)
{
  const auto global_pos = viewport_to_global_position(point2vec(event->pos()));
  m_scene.tool_box.active_tool().mouse_release(global_pos, *event);
  if (event->button() == Qt::RightButton) {
    auto menu = m_scene.tool_box.active_tool().make_context_menu(this);
    if (menu) { menu->exec(event->globalPos()); }
  }
  QWidget::mouseReleaseEvent(event);
}

ObjectTransformation Viewport::viewport_transformation() const
{
  const auto center = arma::vec2{ static_cast<double>(width()) / 2.0,
                                  static_cast<double>(height()) / 2.0 };
  return m_viewport_transformation.translated(center);
}

Scene& Viewport::scene() const
{
  return m_scene;
}

}  // namespace omm
