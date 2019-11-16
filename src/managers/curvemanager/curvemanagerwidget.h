#pragma once

#include <QWidget>
#include "managers/curvemanager/curvetimelinecanvas.h"
#include "managers/timeline/timelinecanvas.h"
#include "managers/range.h"

namespace omm
{

class Manager;

class CurveManagerWidget : public QWidget
{
  Q_OBJECT
public:
  explicit CurveManagerWidget(Scene& scene);

protected:
  void paintEvent(QPaintEvent* event);

  struct ValueRange : Range
  {
    ValueRange(QWidget& canvas) : Range(-100, 100), m_canvas(canvas) {}
    int pixel_range() const override { return m_canvas.height(); }
  private:
    QWidget& m_canvas;
  } value_range;

  struct FrameRange : Range
  {
    FrameRange(QWidget& canvas) : Range(1, 100), m_canvas(canvas) {}
    int pixel_range() const override { return m_canvas.width(); }
  private:
    QWidget& m_canvas;
  } frame_range;

};

}  // namespace omm
