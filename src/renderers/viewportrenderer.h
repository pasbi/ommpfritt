#pragma once

#include "renderers/abstractrenderer.h"

class QPainter;

namespace omm
{

class ViewportRenderer : public AbstractRenderer
{
public:
  explicit ViewportRenderer(QPainter& painter, const Region& region);
  void draw_spline(const std::vector<Point>& points, const Style& style) override;

private:
  QPainter& m_painter;
};

}  // namespace omm
