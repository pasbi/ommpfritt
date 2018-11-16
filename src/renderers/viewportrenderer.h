#pragma once

#include "renderers/abstractrenderer.h"

class QPainter;

namespace omm
{

class ViewportRenderer : public AbstractRenderer
{
public:
  explicit ViewportRenderer(QPainter& painter, const BoundingBox& bounding_box);
  void draw_spline(const std::vector<Point>& points, const Style& style) override;

  void push_transformation(const ObjectTransformation& transformation) override;
  void pop_transformation() override;

private:
  QPainter& m_painter;
};

}  // namespace omm
