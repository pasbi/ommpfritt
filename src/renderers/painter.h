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
  struct Options
  {
    std::vector<const Style*> styles;
    const Style* default_style = nullptr;
    bool viewport = true;
  };

  enum class Category { None = 0x0, Objects = 0x1, Handles = 0x2, BoundingBox = 0x4,
                        All = Objects | Handles | BoundingBox };
  explicit Painter(Scene& scene, Category filter);
  void render(Options options);

  void push_transformation(const ObjectTransformation& transformation);
  void pop_transformation();
  ObjectTransformation current_transformation() const;

  void toast(const Vec2f& pos, const QString& text);

  static QPainterPath path(const std::vector<Point>& points, bool closed = false);
  static QBrush make_brush(const Style& style);
  static QPen make_pen(const Style& style);

  void set_style(const Style& style);

  Scene& scene;
  Category category_filter;
  QPainter* painter = nullptr;
  ImageCache image_cache;

private:
  std::stack<ObjectTransformation> m_transformation_stack;

  int reference_depth = 0;
  friend class ReferenceDepthGuard;
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::Painter::Category> : std::true_type {};
