#include "mainwindow/viewport/viewport.h"

#include <glog/logging.h>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "renderers/viewportrenderer.h"

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
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(&*m_timer, &QTimer::timeout, [this]() {
    update();
  });
  m_timer->setInterval(30);
  m_timer->start();
}

void Viewport::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(rect(), Qt::gray);

  const auto width = static_cast<double>(this->width());
  const auto height = static_cast<double>(this->height());
  const BoundingBox render_box({ { 0, 0 },
                                 { width, height } });
  ViewportRenderer renderer(painter, render_box);
  auto viewport_transformation = m_viewport_transformation;
  viewport_transformation.set_translation(  viewport_transformation.translation()
                                          + arma::vec2{ width/2, height/2} );
  renderer.set_base_transformation(viewport_transformation);
  renderer.render(m_scene);
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
  m_pan_controller.init(point2vec(event->pos()));

  if (event->modifiers() & Qt::AltModifier) {
    event->accept();
  } else {
    QWidget::mousePressEvent(event);
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
  if (event->modifiers() & Qt::AltModifier ) {
    const auto widget_size = arma::vec2{ static_cast<double>(width()),
                                         static_cast<double>(height()) };
    const auto cursor_position = point2vec(event->pos());
    const auto delta = m_pan_controller.delta(cursor_position, widget_size);
    m_viewport_transformation.translate(delta);
    event->accept();
  } else {
    QWidget::mouseMoveEvent(event);
  }
}

}  // namespace omm
