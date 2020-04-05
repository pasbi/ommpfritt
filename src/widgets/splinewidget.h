#pragma once

#include <QWidget>
#include "cachedgetter.h"
#include "splinetype.h"

namespace omm
{

class SplineWidget : public QWidget
{
public:
  SplineWidget(QWidget* parent = nullptr);

  void set_model(SplineType* spline);

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

private:
  SplineType* m_model = nullptr;

  void draw_spline(QPainter& painter);

  QTransform transform() const;
  SplineType::ControlPoint m_grabbed_knot;
  SplineType::ControlPoint knot_at(const QPoint& pos) const;
};

}  // namespace omm
