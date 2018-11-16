#include "mainwindow/viewport.h"

#include <glog/logging.h>
#include <QPainter>
#include <QTimer>

#include "renderers/viewportrenderer.h"

namespace omm
{

Viewport::Viewport(Scene& scene, QWidget* parent)
  : QWidget(parent)
  , m_scene(scene)
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

  const BoundingBox render_box({ { 0, 0 },
                                 { double(width()), double(height()) } });
  ViewportRenderer(painter, render_box).render(m_scene);
}

void Viewport::mouseDoubleClickEvent(QMouseEvent* event)
{
  // TODO remove this function once viewport auto update works reliably.
  update();
  QWidget::mouseDoubleClickEvent(event);
}

}  // namespace omm
