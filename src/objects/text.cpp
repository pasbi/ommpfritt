#include "objects/text.h"
#include "mainwindow/viewport/viewport.h"
#include "path/pathvector.h"
#include "properties/floatproperty.h"
#include "properties/stringproperty.h"
#include "renderers/painter.h"
#include <QFont>
#include <QObject>

namespace omm
{
class Style;

Text::Text(Scene* scene)
    : Object(scene), m_font_properties("", *this), m_text_option_properties("", *this)
{
  static constexpr double DEFAULT_WIDTH = 200.0;
  static const auto text_category = QObject::tr("Text");
  create_property<StringProperty>(TEXT_PROPERTY_KEY, "Text")
      .set_mode(StringProperty::Mode::MultiLine)
      .set_label(QObject::tr("Text"))
      .set_category(text_category);
  create_property<FloatProperty>(WIDTH_PROPERTY_KEY, DEFAULT_WIDTH)
      .set_label(QObject::tr("Width"))
      .set_category(text_category);

  m_font_properties.make_properties(QObject::tr("Font"));
  m_text_option_properties.make_properties(QObject::tr("Text"));
  update();
}

Text::Text(const Text& other)
    : Object(other), m_font_properties("", *this), m_text_option_properties("", *this)
{
}

BoundingBox Text::bounding_box(const ObjectTransformation& transformation) const
{
  if (is_active()) {
    const std::set ps{Vec2f(0, 0), Vec2f(100, 100)};
    // TODO something smarter would be great.
    return BoundingBox(util::transform(ps, [&transformation](const Vec2f& v) {
      return transformation.apply_to_position(v);
    }));
  } else {
    return BoundingBox{};
  }
}

QString Text::type() const
{
  return TYPE;
}

Flag Text::flags() const
{
  return Flag::None;

  // Flag::Convertible  // TODO in future, the text should be convertible to a path.
  // currently this is a tough problem because paths cannot have multiple segments.
  // either implement multi-segment-paths or create many paths in a hierarchy.

  // Flag::IsPathLike   // Maybe this becomes feasible once the convert-trait is implemented.
}

void Text::draw_object(Painter& renderer, const Style& style, const PainterOptions& options) const
{
  Q_UNUSED(options)
  if (is_active()) {
    const QFont font = m_font_properties.get_font();
    const QTextOption option = m_text_option_properties.get_option();
    renderer.painter->setFont(font);
    renderer.set_style(style, *this, options);

    const QRectF rect = this->rect(option.alignment());
    const QString text = property(TEXT_PROPERTY_KEY)->value<QString>();
    renderer.painter->drawText(rect, text, option);
  }
}

QRectF Text::rect(Qt::Alignment alignment) const
{
  static constexpr double HUGE_NUMBER = 10e10;
  const double base_width = property(WIDTH_PROPERTY_KEY)->value<double>();
  const auto [left, width] = [&alignment, base_width]() {
    switch (alignment & Qt::AlignHorizontal_Mask) {
    case Qt::AlignLeft:
      [[fallthrough]];
    case Qt::AlignJustify:
      return std::pair(0.0, base_width);
    case Qt::AlignHCenter:
      return std::pair(-base_width / 2.0, base_width);
    case Qt::AlignRight:
      return std::pair(-base_width, base_width);
    default:
      assert(false);
      return std::pair(0.0, 0.0);
    }
  }();

  const auto [top, height] = [&alignment]() {
    switch (alignment & Qt::AlignVertical_Mask) {
    case Qt::AlignTop:
      return std::pair(0.0, HUGE_NUMBER);
    case Qt::AlignVCenter:
      return std::pair(-HUGE_NUMBER / 2.0, HUGE_NUMBER);
    case Qt::AlignBottom:
      return std::pair(-HUGE_NUMBER, HUGE_NUMBER);
    // Qt::AlignBaseline is never reached (see @FontProperties::code make_properties)
    case Qt::AlignBaseline:
    default:
      assert(false);
      return std::pair(0.0, 0.0);
    }
  }();

  return {QPointF(left, top), QSizeF(width, height)};
}

PathVector Text::compute_path_vector() const
{
  return {};
}

void Text::on_property_value_changed(Property* property)
{
  if (property == this->property(TEXT_PROPERTY_KEY)
      || property == this->property(WIDTH_PROPERTY_KEY)
      || property == this->property(TextOptionProperties::ALIGNH_PROPERTY_KEY)
      || property == this->property(TextOptionProperties::ALIGNV_PROPERTY_KEY)
      || property == this->property(TextOptionProperties::DIRECTION_PROPERTY_KEY)
      || property == this->property(TextOptionProperties::WRAP_MODE_PROPERTY_KEY)
      || property == this->property(TextOptionProperties::TABSTOPDISTANCE_PROPERTY_KEY)
      || property == this->property(FontProperties::FONT_PROPERTY_KEY)
      || property == this->property(FontProperties::SIZE_PROPERTY_KEY)
      || property == this->property(FontProperties::ITALIC_PROPERTY_KEY)
      || property == this->property(FontProperties::WEIGHT_PROPERTY_KEY)
      || property == this->property(FontProperties::KERNING_PROPERTY_KEY)
      || property == this->property(FontProperties::OVERLINE_PROPERTY_KEY)
      || property == this->property(FontProperties::STRIKEOUT_PROPERTY_KEY)
      || property == this->property(FontProperties::UNDERLINE_PROPERTY_KEY)
      || property == this->property(FontProperties::FIXED_PITCH_PROPERTY_KEY)
      || property == this->property(FontProperties::WORD_SPACING_PROPERTY_KEY)
      || property == this->property(FontProperties::CAPITALIZATION_PROPERTY_KEY)
      || property == this->property(FontProperties::LETTER_SPACING_PROPERTY_KEY)
      || property == this->property(FontProperties::LETTER_SPACING_TYPE_PROPERTY_KEY)) {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

}  // namespace omm
