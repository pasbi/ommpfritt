#include "mainwindow/viewport.h"

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

}  // namespace

namespace omm
{

Viewport::Viewport(Scene& scene)
  : m_scene(scene)
  , m_timer(std::make_unique<QTimer>())
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
  MousePanArea::init();

  if (event->modifiers() & Qt::AltModifier) {
    event->accept();
  } else {
    QWidget::mousePressEvent(event);
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
  if (event->modifiers() & Qt::AltModifier ) {
    const auto delta = MousePanArea::delta(point2vec(event->pos()));
    m_viewport_transformation.translate(delta);
    event->accept();
  } else {
    QWidget::mouseMoveEvent(event);
  }
}

void MousePanArea::init()
{
  m_last_position = point2vec(QWidget::mapFromGlobal(cursor().pos()));
}

arma::vec2 MousePanArea::delta(arma::vec2 new_position)
{
  auto cursor = QWidget::cursor();
  const decltype(m_last_position) delta = new_position - m_last_position;

  const auto infinity_pan_shift = [](double& coordinate, const double max) {
    while (coordinate >= max) {
      coordinate -= max;
    }
    while (coordinate <= 0) {
      coordinate += max;
    }
  };

  infinity_pan_shift(new_position(0), width());
  infinity_pan_shift(new_position(1), height());

  cursor.setPos(QWidget::mapToGlobal(QPoint(new_position(0), new_position(1))));
  // QWidget::setCursor(cursor);
  m_last_position = new_position;

  return delta;
}

}  // namespace omm
