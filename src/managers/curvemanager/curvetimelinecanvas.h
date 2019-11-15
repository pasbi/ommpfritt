#pragma once

#include "managers/timeline/timelinecanvas.h"

namespace omm
{

class CurveTimelineCanvas : public TimelineCanvas
{
public:
  CurveTimelineCanvas(Animator& animator, QWidget& widget);
  void draw_lines(QPainter& painter) const;
  void draw_keyframes(QPainter& painter) const;

  double ppv() const;
  double normalized_to_value(double normalized) const;

  struct ValueRange : Range {
    ValueRange(CurveTimelineCanvas& self) : Range(-100, 100), m_self(self) {}
    int pixel_range() const override { return m_self.rect.height(); }
  private:
    CurveTimelineCanvas& m_self;
  } value_range;


protected:
  void pan(const QPointF& d) override;
  void zoom(const QPointF& d) override;

private:
  void draw_keyframe(QPainter& painter, Track& track, int keyframe, std::size_t i) const;
  void draw_segment(QPainter& painter, Track& track, int kf1, int kf2, std::size_t i) const;
};

}  // namespace omm
