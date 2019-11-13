#include "managers/curvemanager/curvemanagerwidget.h"
#include <QPainter>
#include <QEvent>
#include "managers/manager.h"
#include "scene/scene.h"

namespace omm
{

CurveManagerWidget::CurveManagerWidget(Scene& scene)
  : m_canvas(scene.animator(), *this)
{
}

void CurveManagerWidget::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  m_canvas.rect = rect();

  painter.save();
  m_canvas.draw_background(painter);
  m_canvas.draw_lines(painter);
  m_canvas.draw_keyframes(painter);
  m_canvas.draw_current(painter);
  m_canvas.draw_rubber_band(painter);
  painter.restore();
  QWidget::paintEvent(event);
}

bool CurveManagerWidget::event(QEvent* event)
{
  if (m_canvas.view_event(*event)) {
    event->accept();
    return true;
  } else {
    return QWidget::event(event);
  }
}

CurveManagerWidget::TimelineCanvasC::TimelineCanvasC(Animator& animator, QWidget& widget)
  : CurveTimelineCanvas(animator, widget), m_self(widget)
{

}

void CurveManagerWidget::TimelineCanvasC::disable_context_menu()
{
  m_self.setContextMenuPolicy(Qt::PreventContextMenu);
}

void CurveManagerWidget::TimelineCanvasC::enable_context_menu()
{
  m_self.setContextMenuPolicy(Qt::DefaultContextMenu);
}

QPoint CurveManagerWidget::TimelineCanvasC::map_to_global(const QPoint& pos) const
{
  return m_self.mapToGlobal(pos);
}

void CurveManagerWidget::TimelineCanvasC::update()
{
  m_self.update();
}

QRect CurveManagerWidget::TimelineCanvasC::track_rect(Track& track)
{
  Q_UNUSED(track)
  return QRect();
}

QRect CurveManagerWidget::TimelineCanvasC::owner_rect(AbstractPropertyOwner& owner)
{
  Q_UNUSED(owner)
  return QRect();
}

}  // namespace omm
