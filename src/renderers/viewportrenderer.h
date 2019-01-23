#pragma once

#include "renderers/abstractrenderer.h"

class QPainter;

namespace omm
{

class ViewportRenderer : public AbstractRenderer
{
public:
  explicit ViewportRenderer(QPainter& painter, Scene& scene);
  void draw_spline( const std::vector<Point>& points, const Style& style,
                    bool closed = false ) override;
  void draw_rectangle(const arma::vec2& pos, const double radius, const Style& style) override;
  void draw_circle(const arma::vec2& pos, const double radius, const Style& style) override;

  void push_transformation(const ObjectTransformation& transformation) override;
  void pop_transformation() override;

private:
  QPainter& m_painter;
};

}  // namespace omm
