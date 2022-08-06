#include "properties/propertygroups/markerproperties.h"
#include "objects/tip.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/optionproperty.h"
#include "path/path.h"
#include "renderers/painter.h"
#include <QPainter>

namespace omm
{
MarkerProperties ::MarkerProperties(const QString& prefix,
                                    omm::AbstractPropertyOwner& property_owner,
                                    const Shape default_shape,
                                    const double default_size)
    : PropertyGroup(prefix, property_owner), m_default_shape(default_shape),
      m_default_size(default_size)
{
}

void MarkerProperties::make_properties(const QString& category) const
{
  static constexpr double SIZE_STEP = 0.1;
  static constexpr double ASPECT_RATIO_STEP = 0.001;
  create_property<OptionProperty>(SHAPE_PROPERTY_KEY, static_cast<std::size_t>(m_default_shape))
      .set_options(shapes())
      .set_category(category)
      .set_label(QObject::tr("Shape"));

  create_property<FloatProperty>(SIZE_PROPERTY_KEY, m_default_size)
      .set_step(SIZE_STEP)
      .set_range(0.0, FloatProperty::highest_possible_value())
      .set_label(QObject::tr("Size"))
      .set_category(category);

  create_property<FloatProperty>(ASPECT_RATIO_PROPERTY_KEY)
      .set_step(ASPECT_RATIO_STEP)
      .set_label(QObject::tr("Aspect Ratio"))
      .set_category(category);

  create_property<BoolProperty>(REVERSE_PROPERTY_KEY)
      .set_label(QObject::tr("Reverse"))
      .set_category(category);
}

void MarkerProperties ::draw_marker(Painter& painter,
                                    const Point& location,
                                    const Color& color,
                                    const double width) const
{
  QPainter& p = *painter.painter;
  p.save();
  p.translate(location.position().to_pointf());
  if (property_value<bool>(REVERSE_PROPERTY_KEY)) {
    p.rotate((location.rotation() + M_PI_2) * M_180_PI);
  } else {
    p.rotate((location.rotation() - M_PI_2) * M_180_PI);
  }
  p.setPen(Qt::NoPen);
  p.setBrush(color.to_qcolor());
  p.drawPath(shape(width));
  p.restore();
}

std::deque<QString> MarkerProperties::shapes()
{
  return {QObject::tr("None"),
          QObject::tr("Arrow"),
          QObject::tr("Bar"),
          QObject::tr("Circle"),
          QObject::tr("Diamond")};
}

QPainterPath MarkerProperties::shape(const double width) const
{
  const double base = width * property_value<double>(SIZE_PROPERTY_KEY);
  const auto shape = property_value<Shape>(SHAPE_PROPERTY_KEY);
  const auto ar = property_value<double>(ASPECT_RATIO_PROPERTY_KEY);

  const Vec2f size(base, base * std::exp(ar));
  switch (shape) {
  case Shape::None:
    return {};
  case Shape::Arrow:
    return arrow(size);
  case Shape::Bar:
    return bar(size);
  case Shape::Circle:
    return circle(size);
  case Shape::Diamond:
    return diamond(size);
  }
  Q_UNREACHABLE();
}

QPainterPath MarkerProperties::arrow(const Vec2f& size)
{
  QPainterPath p;
  p.moveTo(size.x, 0.0);
  p.lineTo(0.0, size.y);
  p.lineTo(0.0, -size.y);
  p.lineTo(size.x, 0.0);
  return p;
}

QPainterPath MarkerProperties::bar(const Vec2f& size)
{
  QPainterPath p;
  p.moveTo(-size.x, size.y);
  p.lineTo(-size.x, -size.y);
  p.lineTo(size.x, -size.y);
  p.lineTo(size.x, size.y);
  p.lineTo(-size.x, size.y);
  return p;
}

QPainterPath MarkerProperties::circle(const Vec2f& size)
{
  Q_UNUSED(size);
  return {};
}

QPainterPath MarkerProperties::diamond(const Vec2f& size)
{
  QPainterPath p;
  p.moveTo(-size.x, 0.0);
  p.lineTo(0.0, -size.y);
  p.lineTo(size.x, 0.0);
  p.lineTo(0.0, size.y);
  p.lineTo(-size.x, 0.0);
  return p;
}

}  // namespace omm
