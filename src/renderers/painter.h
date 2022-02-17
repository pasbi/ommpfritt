#pragma once

#include <stack>
#include <string>

#include "color/color.h"
#include "common.h"
#include "geometry/objecttransformation.h"
#include "renderers/imagecache.h"
#include <QPainter>
#include <QPainterPath>

class QFont;
class QTextOption;

namespace omm
{

class Object;
class Point;
class Rectangle;
class Scene;
class Style;
class Viewport;
struct PainterOptions;

class Painter
{
public:
  enum class Category {
    None = 0x0,
    Objects = 0x1,
    Handles = 0x2,
    BoundingBox = 0x4,
    All = Objects | Handles | BoundingBox
  };
  explicit Painter(const Scene& scene, Category filter);
  void render(const PainterOptions& options);

  void push_transformation(const ObjectTransformation& transformation);
  void pop_transformation();
  ObjectTransformation current_transformation() const;

  void toast(const Vec2f& pos, const QString& text) const;

  static QBrush make_brush(const Style& style,
                           const Object& object,
                           const PainterOptions& options);
  static QPen make_pen(const Style& style, const Object& object);

  static QBrush make_simple_brush(const Style& style);
  static QPen make_simple_pen(const Style& style);

  void set_style(const Style& style, const Object& object, const PainterOptions& options) const;

  const Scene& scene;
  Category category_filter;
  QPainter* painter = nullptr;
  ImageCache image_cache;

private:
  std::stack<ObjectTransformation> m_transformation_stack;

  int reference_depth = 0;
  friend class ReferenceDepthGuard;
};

}  // namespace omm

template<> struct omm::EnableBitMaskOperators<omm::Painter::Category> : std::true_type {
};
