#include "renderers/painter.h"
#include "geometry/util.h"
#include "scene/scene.h"

namespace
{

QTransform to_transformation(const omm::ObjectTransformation& transformation)
{
  const auto& m = transformation.to_mat();
  return QTransform( m.m[0][0], m.m[1][0], m.m[2][0],
                     m.m[0][1], m.m[1][1], m.m[2][1],
                     m.m[0][2], m.m[1][2], m.m[2][2] );
}

}  // namespace

namespace omm
{

omm::Painter::Painter(omm::Scene &scene, omm::Painter::Category filter)
  : scene(scene), category_filter(filter)
{

}

void Painter::render()
{
  scene.object_tree.root().draw_recursive(*this, scene.default_style());
  assert(m_transformation_stack.empty());
}

void Painter::push_transformation(const ObjectTransformation &transformation)
{
  m_transformation_stack.push(current_transformation().apply(transformation));
  painter->setTransform(to_transformation(current_transformation()), false);
}

void Painter::pop_transformation()
{
  m_transformation_stack.pop();
  painter->setTransform(to_transformation(current_transformation()), false);
}

ObjectTransformation Painter::current_transformation() const
{
  if (m_transformation_stack.size() == 0) {
    return ObjectTransformation();
  } else {
    return m_transformation_stack.top();
  }
}

void Painter::draw_text(const std::string &text, const Painter::TextOptions &options)
{
  painter->setFont(options.font);
  painter->setPen(make_pen(options.style));

  static constexpr double HUGE_NUMBER = 10e10;

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
    case Qt::AlignTop: return std::pair(0.0, HUGE_NUMBER);
    case Qt::AlignVCenter: return std::pair(-HUGE_NUMBER/2.0, HUGE_NUMBER);
    case Qt::AlignBottom: return std::pair(-HUGE_NUMBER, HUGE_NUMBER);
    // Qt::AlignBaseline is never reached (see @FontProperties::code make_properties)
    case Qt::AlignBaseline:
    default: assert(false); return std::pair(0.0, 0.0);
    }
  }();

  const QRectF rect(QPointF(left, top), QSizeF(width, height));
  painter->drawText(rect, QString::fromStdString(text), options.option);
}

void Painter::toast(const Vec2f &pos, const std::string &text)
{
  static const QFont toast_font("Helvetica", 12, 0, false);
  static const QPen pen(Qt::black, 1.0);
  painter->save();
  painter->setFont(toast_font);
  painter->setPen(pen);
  const Vec2f gpos = current_transformation().apply_to_position(pos);
  painter->resetTransform();
  const QPointF top_left = to_qpoint(gpos);
  static constexpr double huge = 10.0e10;
  const QRectF rect(top_left, QSizeF(huge, huge));
  QRectF actual_rect;
  painter->drawText(rect, Qt::AlignTop | Qt::AlignLeft, text.c_str(), &actual_rect);
  const double margin = 10.0;
  actual_rect.adjust(-margin, -margin, margin, margin);

  painter->setBrush(QBrush(QColor(80, 60, 40, 120)));
  painter->setPen(QPen(Qt::white));
  painter->drawRoundRect(actual_rect);
  painter->restore();
}

void Painter::draw_image(const std::string &filename, const double opacity)
{
}

QPainterPath Painter::path(const std::vector<Point> &points, bool closed)
{
  QPainterPath path;
  if (points.size() > 1) {
    path.moveTo(to_qpoint(points.front().position));

    for (size_t i = 1; i < points.size(); ++i)
    {
      path.cubicTo( to_qpoint(points.at(i-1).right_position()),
                    to_qpoint(points.at(i).left_position()),
                    to_qpoint(points.at(i).position)  );
    }

    if (closed && points.size() > 2) {
      path.cubicTo(to_qpoint(points.back().right_position()),
                   to_qpoint(points.front().left_position()),
                   to_qpoint(points.front().position) );
    }

  }
  return path;
}

QBrush Painter::make_brush(const Style &style)
{
  if (style.property(omm::Style::BRUSH_IS_ACTIVE_KEY)->value<bool>()) {
    QBrush brush(Qt::SolidPattern);
    const auto color = style.property(omm::Style::BRUSH_COLOR_KEY)->value<omm::Color>();
    brush.setColor(to_qcolor(color));
    return brush;
  } else {
    return QBrush(Qt::NoBrush);
  }
}

QPen Painter::make_pen(const Style &style)
{
  if (style.property(omm::Style::PEN_IS_ACTIVE_KEY)->value<bool>()) {
    QPen pen;
    pen.setWidthF(style.property(omm::Style::PEN_WIDTH_KEY)->value<double>());
    pen.setColor(to_qcolor(style.property(omm::Style::PEN_COLOR_KEY)->value<omm::Color>()));
    pen.setCosmetic(style.property(omm::Style::COSMETIC_KEY)->value<bool>());
    switch (style.property(omm::Style::CAP_STYLE_KEY)->value<std::size_t>()) {
    case 0: pen.setCapStyle(Qt::SquareCap); break;
    case 1: pen.setCapStyle(Qt::FlatCap); break;
    case 2: pen.setCapStyle(Qt::RoundCap); break;
    }
    switch (style.property(omm::Style::JOIN_STYLE_KEY)->value<std::size_t>()) {
    case 0: pen.setJoinStyle(Qt::BevelJoin); break;
    case 1: pen.setJoinStyle(Qt::MiterJoin); break;
    case 2: pen.setJoinStyle(Qt::RoundJoin); break;
    }
    const auto pen_style = style.property(omm::Style::STROKE_STYLE_KEY)->value<Qt::PenStyle>();
    pen.setStyle(static_cast<Qt::PenStyle>(pen_style + 1));
    return pen;
  } else {
    return QPen(Qt::NoPen);
  }
}

QColor Painter::to_qcolor(Color color)
{
  color = color.clamped() * 255.0;
  return QColor( static_cast<int>(color.red()),
                 static_cast<int>(color.green()),
                 static_cast<int>(color.blue()),
                 static_cast<int>(color.alpha()) );
}

void Painter::set_style(const Style &style)
{
  painter->setPen(make_pen(style));
  painter->setBrush(make_brush(style));
}

}  // namespace omm
