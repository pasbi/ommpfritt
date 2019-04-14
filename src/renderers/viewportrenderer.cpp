#include "renderers/viewportrenderer.h"

#include <QTransform>
#include <QPainter>
#include <QDebug>

#include "renderers/style.h"
#include "properties/colorproperty.h"
#include "properties/floatproperty.h"
#include "properties/boolproperty.h"
#include "geometry/util.h"

namespace omm
{

ViewportRenderer::ViewportRenderer(Scene& scene, Category filter)
  : AbstractRenderer(scene, filter)
{
}

void ViewportRenderer::push_transformation(const ObjectTransformation& transformation)
{
  AbstractRenderer::push_transformation(transformation);
  m_painter->setTransform(to_transformation(current_transformation()), false);
}

void ViewportRenderer::pop_transformation()
{
  AbstractRenderer::pop_transformation();
  m_painter->setTransform(to_transformation(current_transformation()), false);
}

void
ViewportRenderer::draw_spline(const std::vector<Point>& points, const Style& style, bool closed)
{
  if (points.size() > 1) {
    QPainterPath path = to_path(points, closed);

    if (style.property(Style::BRUSH_IS_ACTIVE_KEY).value<bool>()) {
      m_painter->fillPath(path, make_brush(style));
    }
    if (style.property(Style::PEN_IS_ACTIVE_KEY).value<bool>()) {
      m_painter->strokePath(path, make_pen(style));
    }
  }
}

void ViewportRenderer::draw_text(const std::string& text, const TextOptions& options)
{
  m_painter->setFont(options.font);
  m_painter->setPen(make_pen(options.style));

  static constexpr double HUGE = 10e10;

  const auto [left, width] = [&options]() {
    switch (options.option.alignment() & Qt::AlignHorizontal_Mask) {
    case Qt::AlignLeft: [[fallthrough]];
    case Qt::AlignJustify: return std::pair(0.0, options.width);
    case Qt::AlignHCenter: return std::pair(-options.width/2.0, options.width);
    case Qt::AlignRight: return std::pair(-options.width, options.width);
    default: assert(false); return std::pair(0.0, 0.0);
    }
  }();

  const auto [top, height] = [&options]() {
    switch (options.option.alignment() & Qt::AlignVertical_Mask) {
    case Qt::AlignTop: return std::pair(0.0, HUGE);
    case Qt::AlignVCenter: return std::pair(-HUGE/2.0, HUGE);
    case Qt::AlignBottom: return std::pair(-HUGE, HUGE);
    // Qt::AlignBaseline is never reached (see @FontProperties::code make_properties)
    case Qt::AlignBaseline:
    default: assert(false); return std::pair(0.0, 0.0);
    }
  }();

  const QRectF rect(QPointF(left, top), QSizeF(width, height));
  m_painter->drawText(rect, QString::fromStdString(text), options.option);
}

void ViewportRenderer::draw_rectangle(const Rectangle &rect, const Style &style)
{
  draw_spline(rect.to_points(), style, true);
}

void ViewportRenderer::draw_circle(const Vec2f& pos, const double radius, const Style& style)
{
  m_painter->save();
  m_painter->setPen(make_pen(style));
  m_painter->setBrush(make_brush(style));
  m_painter->drawEllipse(to_qpoint(pos), radius, radius);
  m_painter->restore();
}

void ViewportRenderer::draw_image( const std::string& filename, const Vec2f& pos,
                                   const Vec2f& size, const double opacity )
{
  m_painter->setOpacity(opacity);
  QRectF rect(to_qpoint(pos), to_qpoint(pos + size));
  m_painter->drawImage(rect, m_image_cache.load(filename));
  m_painter->setOpacity(1.0);
}

void ViewportRenderer::draw_image( const std::string& filename, const Vec2f& pos,
                                   const double& width, const double opacity )
{
  const QImage image = m_image_cache.load(filename);
  const auto height = static_cast<double>(width) / image.width() * image.height();
  return draw_image(filename, pos, Vec2f{ width, height }, opacity);
}


void ViewportRenderer::set_painter(QPainter& painter) { m_painter = &painter; }
void ViewportRenderer::clear_painter() { m_painter = nullptr; }


QTransform ViewportRenderer::to_transformation(const omm::ObjectTransformation& transformation)
{
  const auto& m = transformation.to_mat();
  return QTransform( m.m[0][0], m.m[1][0], m.m[2][0],
                     m.m[0][1], m.m[1][1], m.m[2][1],
                     m.m[0][2], m.m[1][2], m.m[2][2] );
}

QColor ViewportRenderer::to_qcolor(omm::Color color)
{
  color = color.clamped() * 255.0;
  return QColor( static_cast<int>(color.red()),
                 static_cast<int>(color.green()),
                 static_cast<int>(color.blue()),
                 static_cast<int>(color.alpha()) );
}

QPen ViewportRenderer::make_pen(const omm::Style& style)
{
  if (style.property(omm::Style::PEN_IS_ACTIVE_KEY).value<bool>()) {
    QPen pen;
    pen.setWidthF(style.property(omm::Style::PEN_WIDTH_KEY).value<double>());
    pen.setColor(to_qcolor(style.property(omm::Style::PEN_COLOR_KEY).value<omm::Color>()));
    return pen;
  } else {
    return QPen(Qt::NoPen);
  }
}

QBrush ViewportRenderer::make_brush(const omm::Style& style)
{
  if (style.property(omm::Style::BRUSH_IS_ACTIVE_KEY).value<bool>()) {
    QBrush brush(Qt::SolidPattern);
    const auto color = style.property(omm::Style::BRUSH_COLOR_KEY).value<omm::Color>();
    brush.setColor(to_qcolor(color));
    return brush;
  } else {
    return QBrush(Qt::NoBrush);
  }
}

void ViewportRenderer::toast(const Vec2f& pos, const std::string& text)
{
  m_painter->save();
  const Vec2f gpos = current_transformation().apply_to_position(pos);
  m_painter->resetTransform();
  const QPointF top_left = to_qpoint(gpos);
  static constexpr double huge = 10.0e10;
  const QRectF rect(top_left, QSizeF(huge, huge));
  QRectF actual_rect;
  m_painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft, text.c_str(), &actual_rect);
  const double margin = 10.0;
  actual_rect.adjust(-margin, -margin, margin, margin);

  m_painter->setBrush(QBrush(QColor(80, 60, 40, 120)));
  m_painter->setPen(QPen(Qt::white));
  m_painter->drawRoundRect(actual_rect);
  m_painter->restore();
}

}  // namespace omm
