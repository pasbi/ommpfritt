#include "managers/dopesheetmanager/dopesheetheader.h"
#include "animation/animator.h"
#include "logging.h"
#include "managers/timeline/timelinecanvas.h"

#include <QEvent>
#include <QMouseEvent>

namespace omm
{
DopeSheetHeader::DopeSheetHeader(TimelineCanvas& canvas)
    : QHeaderView(Qt::Horizontal), m_canvas(canvas)
{
  setStretchLastSection(true);
}

void DopeSheetHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
  if (logicalIndex == 1) {
    activate();
    painter->setClipRect(rect);
    m_canvas.rect = rect;
    m_canvas.footer_height = rect.height();
    m_canvas.draw_background(*painter);
    m_canvas.draw_lines(*painter);
    m_canvas.draw_current(*painter);
  }
}

void DopeSheetHeader::mouseMoveEvent(QMouseEvent* e)
{
  if (logicalIndexAt(e->pos()) == 1) {
    activate();
    m_canvas.view_event(*e);
  }
}

void DopeSheetHeader::mouseReleaseEvent(QMouseEvent* e)
{
  if (logicalIndexAt(e->pos()) == 1) {
    activate();
    m_canvas.view_event(*e);
  }
}

void DopeSheetHeader::mousePressEvent(QMouseEvent* e)
{
  if (logicalIndexAt(e->pos()) == 1) {
    activate();
    m_canvas.view_event(*e);
  }
}

void DopeSheetHeader::activate() const
{
  m_canvas.rect = QRectF(QPointF(sectionPosition(1), 0), QSizeF(sectionSize(1), height()));
}

}  // namespace omm
