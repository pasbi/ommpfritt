#pragma once

#include "renderers/abstractrenderer.h"
#include "renderers/imagecache.h"
#include "color/color.h"

class QPainter;

namespace omm
{

class ViewportRenderer : public AbstractRenderer
{
public:
  explicit ViewportRenderer(Scene& scene, Category filter);
  void draw_spline( const std::vector<Point>& points, const Style& style,
                    bool closed = false ) override;
  void draw_rectangle(const arma::vec2& pos, const double radius, const Style& style) override;
  void draw_circle(const arma::vec2& pos, const double radius, const Style& style) override;
  void
  draw_image(const std::string& filename, const arma::vec2& pos, const arma::vec2& size) override;
  void draw_text(const std::string& text, const TextOptions& options) override;


  void push_transformation(const ObjectTransformation& transformation) override;
  void pop_transformation() override;
  void set_painter(QPainter& painter);
  void clear_painter();

  static QTransform to_transformation(const omm::ObjectTransformation& transformation);
  static QPointF to_qpoint(const arma::vec2& point);
  static QColor to_qcolor(omm::Color color);
  static QPen make_pen(const omm::Style& style);
  static QBrush make_brush(const omm::Style& style);

private:
  QPainter* m_painter;
  ImageCache m_image_cache;
};

}  // namespace omm
