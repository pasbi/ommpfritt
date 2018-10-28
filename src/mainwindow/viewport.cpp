#include "mainwindow/viewport.h"

#include <QPainter>

namespace omm
{
  Viewport::Viewport(Project& project, QWidget* parent)
    : QWidget(parent)
  {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  }

  void Viewport::paintEvent(QPaintEvent* event)
  {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::gray);
  }
}