#pragma once

#include <string>
#include <stack>

#include "geometry/objecttransformation.h"
#include "geometry/boundingbox.h"
#include "geometry/point.h"
#include "common.h"
#include <QPainter>
#include <QPainterPath>
#include "renderers/imagecache.h"
#include "color/color.h"

class QFont;
class QTextOption;

namespace omm
{

class Style;
class Scene;
class Rectangle;

class Painter
{
public:
  struct TextOptions
  {
    TextOptions( const QFont& font, const QTextOption& option,
                 const Style& style, const double width )
      : font(font), option(option), style(style), width(width) {}
    const QFont& font;
    const QTextOption& option;
    const Style& style;
    const double width;
  };

  enum class Category { None = 0x0, Objects = 0x1, Handles = 0x2, BoundingBox = 0x4,
                        All = Objects | Handles | BoundingBox };
  explicit Painter(Scene& scene, Category filter);
  void render();

  void push_transformation(const ObjectTransformation& transformation);
  void pop_transformation();
  ObjectTransformation current_transformation() const;

  void draw_text(const std::string& text, const TextOptions& options);
  void toast(const Vec2f& pos, const std::string& text);

  void draw_image( const std::string& filename, const Vec2f& pos, const Vec2f& size,
                   const double opacity = 1.0);
  void draw_image( const std::string& filename, const Vec2f& pos,
                   const double width, const double opacity);

  static QPainterPath path(const std::vector<Point>& points, bool closed = false);
  static QBrush make_brush(const Style& style);
  static QPen make_pen(const Style& style);
  static QColor to_qcolor(Color color);

  void set_style(const Style& style);

  Scene& scene;
  Category category_filter;
  QPainter* painter = nullptr;

private:
  std::stack<ObjectTransformation> m_transformation_stack;
  ImageCache m_image_cache;
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::Painter::Category> : std::true_type {};
