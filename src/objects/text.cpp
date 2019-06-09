#include "objects/text.h"
#include <QObject>
#include "properties/stringproperty.h"
#include "objects/path.h"
#include <QFont>
#include "mainwindow/viewport/viewport.h"
#include "properties/floatproperty.h"

namespace omm
{

class Style;

Text::Text(Scene* scene)
  : Object(scene), m_font_properties("", *this), m_text_option_properties("", *this)
{
  static const auto text_category = QObject::tr("Text").toStdString();
  add_property<StringProperty>(TEXT_PROPERTY_KEY, "Text" )
    .set_mode(StringProperty::Mode::MultiLine)
    .set_label(QObject::tr("Text").toStdString()).set_category(text_category);
  add_property<FloatProperty>(WIDTH_PROPERTY_KEY, 200)
    .set_label(QObject::tr("Width").toStdString()).set_category(text_category);

  m_font_properties.make_properties(QObject::tr("Font").toStdString());
  m_text_option_properties.make_properties(QObject::tr("Text").toStdString());
}

Text::Text(const Text &other)
  : Object(other), m_font_properties("", *this), m_text_option_properties("", *this)
{
}

BoundingBox Text::bounding_box(const ObjectTransformation &transformation) const
{
  if (is_active()) {
    const std::vector ps { Vec2f(0, 0), Vec2f(100, 100) };
    // TODO something smarter would be great.
    return BoundingBox(::transform<Vec2f>(ps, [&transformation](const Vec2f& v) {
      return transformation.apply_to_position(v);
    }));
  } else {
    return BoundingBox();
  }
}

std::string Text::type() const { return TYPE; }
std::unique_ptr<Object> Text::clone() const { return std::make_unique<Text>(*this); }

AbstractPropertyOwner::Flag Text::flags() const
{
  return Object::flags();

  // Flag::Convertable  // TODO in future, the text should be convertable to a path.
  // currently this is a tough problem because paths cannot have multiple segments.
  // either implement multi-segment-paths or create many paths in a hierarchy.

  // Flag::IsPathLike   // Maybe this becomes feasible once the convert-trait is implemented.
}

void Text::draw_object(Painter &renderer, const Style& style) const
{
  if (is_active()) {
    const QFont font = m_font_properties.get_font();
    const QTextOption option = m_text_option_properties.get_option();
    renderer.painter->setFont(font);
    renderer.set_style(style);

    const QRectF rect = this->rect(option.alignment());
    const std::string text = property(TEXT_PROPERTY_KEY)->value<std::string>();
    renderer.painter->drawText(rect, QString::fromStdString(text), option);
  }
}

QRectF Text::rect(Qt::Alignment alignment) const
{
  static constexpr double HUGE_NUMBER = 10e10;
  const double base_width = property(WIDTH_PROPERTY_KEY)->value<double>();
  const auto [left, width] = [&alignment, base_width]() {
    switch (alignment & Qt::AlignHorizontal_Mask) {
    case Qt::AlignLeft: [[fallthrough]];
    case Qt::AlignJustify: return std::pair(0.0, base_width);
    case Qt::AlignHCenter: return std::pair(-base_width/2.0, base_width);
    case Qt::AlignRight: return std::pair(-base_width, base_width);
    default: assert(false); return std::pair(0.0, 0.0);
    }
  }();

  const auto [top, height] = [&alignment]() {
    switch (alignment & Qt::AlignVertical_Mask) {
    case Qt::AlignTop: return std::pair(0.0, HUGE_NUMBER);
    case Qt::AlignVCenter: return std::pair(-HUGE_NUMBER/2.0, HUGE_NUMBER);
    case Qt::AlignBottom: return std::pair(-HUGE_NUMBER, HUGE_NUMBER);
    // Qt::AlignBaseline is never reached (see @FontProperties::code make_properties)
    case Qt::AlignBaseline:
    default: assert(false); return std::pair(0.0, 0.0);
    }
  }();

  return QRectF(QPointF(left, top), QSizeF(width, height));
}

}  // namespace omm
