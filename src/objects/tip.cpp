#include "objects/tip.h"
#include "properties/optionsproperty.h"
#include "properties/floatproperty.h"

namespace omm
{

Tip::Tip(Scene *scene) : AbstractProceduralPath(scene)
{
  const auto tip_category = QObject::tr("Tip").toStdString();
  add_property<OptionsProperty>(SHAPE_KEY, 1)
      .set_options(shapes()).set_label(QObject::tr("Shape").toStdString())
      .set_category(tip_category);
  add_property<FloatProperty>(BASE_SIZE_KEY, 100.0)
      .set_step(0.1).set_range(0.0, FloatPropertyLimits::upper)
      .set_label(QObject::tr("Size").toStdString()).set_category(tip_category);
  add_property<FloatProperty>(ASPECT_RATIO_KEY, 0.0)
      .set_step(0.001)
      .set_label(QObject::tr("Aspect Ratio").toStdString()).set_category(tip_category);
}

std::vector<Point> Tip::points() const
{
  const double base = property(BASE_SIZE_KEY)->value<double>();
  const double aspect_ratio = property(ASPECT_RATIO_KEY)->value<double>();
  return shape(property(SHAPE_KEY)->value<Shape>(), base, aspect_ratio);
}

std::string Tip::type() const { return "Tip"; }
std::unique_ptr<Object> Tip::clone() const { return std::make_unique<Tip>(*this); }
bool Tip::is_closed() const { return true; }

std::vector<std::string> Tip::shapes()
{
  return { QObject::tr("None").toStdString(),
           QObject::tr("Arrow").toStdString(),
           QObject::tr("Bar").toStdString(),
           QObject::tr("Circle").toStdString(),
           QObject::tr("Diamond").toStdString() };
}

std::vector<Point> Tip::shape(Shape shape, const double base, const double aspect_ratio)
{
  LINFO << aspect_ratio;
  const Vec2f size(base, base * std::exp(aspect_ratio));
  switch (shape) {
  case Shape::None: return {};
  case Shape::Arrow: return arrow(size);
  case Shape::Bar: return bar(size);
  case Shape::Circle: return circle(size);
  case Shape::Diamond: return diamond(size);
  }
  Q_UNREACHABLE();
}

std::vector<Point> Tip::arrow(const Vec2f& size)
{
  LINFO << size;
  return {
    Point(Vec2f(0.0, size.y)),
    Point(Vec2f(size.x, 0.0)),
    Point(Vec2f(-size.x, 0.0)),
  };
}

std::vector<Point> Tip::bar(const Vec2f& size)
{
  return {
    Point(Vec2f(-size.x, size.y)),
    Point(Vec2f(-size.x, -size.y)),
    Point(Vec2f(size.x, -size.y)),
    Point(Vec2f(size.x, size.y)),
  };
}

std::vector<Point> Tip::circle(const Vec2f& size)
{
  Q_UNUSED(size);
  return {};
}

std::vector<Point> Tip::diamond(const Vec2f &size)
{
  return {
    Point(Vec2f(-size.x, 0.0)),
    Point(Vec2f(0.0, -size.y)),
    Point(Vec2f(size.x, 0.0)),
    Point(Vec2f(0.0, size.y)),
  };
}

}  // namespace omm
