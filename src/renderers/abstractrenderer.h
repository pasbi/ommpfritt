#pragma once

#include <string>
#include <stack>

#include "geometry/objecttransformation.h"
#include "geometry/boundingbox.h"
#include "geometry/point.h"
#include "common.h"

class QFont;
class QTextOption;

namespace omm
{

class Style;
class Scene;

class AbstractRenderer
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

  enum class Category { None = 0x0, Objects = 0x1, Handles = 0x2, All = Objects | Handles };
  explicit AbstractRenderer(Scene& scene, Category filter);
  void render();
  const BoundingBox& bounding_box() const;

public:
  virtual void start_group(const std::string& name) {}
  virtual void end_group() {}
  virtual void draw_spline( const std::vector<Point>& points, const Style& style,
                            bool closed = false ) = 0;
  virtual void draw_rectangle(const arma::vec2& pos, const double radius, const Style& style) = 0;
  virtual void push_transformation(const ObjectTransformation& transformation);
  virtual void pop_transformation();
  virtual ObjectTransformation current_transformation() const;
  virtual void draw_circle(const arma::vec2& pos, const double radius, const Style& style) = 0;
  virtual
  void draw_image(const std::string& filename, const arma::vec2& pos, const arma::vec2& size) = 0;
  virtual void draw_text(const std::string& text, const TextOptions& options) = 0;
  Scene& scene;

  void set_category(Category category);
  void end_draw_handles();

protected:
  bool is_active() const;

private:
  std::stack<ObjectTransformation> m_transformation_stack;
  Category m_current_category;
  const Category m_enabled_categories;

};

}  // namespace omm

template<> struct EnableBitMaskOperators<omm::AbstractRenderer::Category> : std::true_type {};

