#include "managers/curvemanager/curvemanagerwidget.h"
#include <QPainter>
#include <QEvent>
#include "managers/manager.h"
#include "scene/scene.h"

namespace omm
{

CurveManagerWidget::CurveManagerWidget(Scene& scene)
  : value_range(*this), frame_range(*this)
{
}

void CurveManagerWidget::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
}

}  // namespace omm
