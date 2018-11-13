#include "mainwindow/viewport.h"

#include <QPainter>

#include "renderers/viewportrenderer.h"

namespace omm
{
  Viewport::Viewport(Scene& scene, QWidget* parent)
    : QWidget(parent)
    , m_scene(scene)
  {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  }

  void Viewport::paintEvent(QPaintEvent* event)
  {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::gray);

    const AbstractRenderer::Region region({ 0, 0 }, { double(width()), double(height()) });
    ViewportRenderer(painter, region).render(m_scene);
  }

}
