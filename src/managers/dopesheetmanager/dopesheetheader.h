#pragma once

#include <QHeaderView>

namespace omm
{
class Animator;
class TimelineCanvas;

class DopeSheetHeader : public QHeaderView
{
public:
  DopeSheetHeader(TimelineCanvas& canvas);

protected:
  void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void activate() const;

private:
  TimelineCanvas& m_canvas;
};

}  // namespace omm
