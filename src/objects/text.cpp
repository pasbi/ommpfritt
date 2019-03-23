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

BoundingBox Text::bounding_box() { return BoundingBox(); }
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

void Text::render(AbstractRenderer& renderer, const Style& style)
{
  renderer.set_category(AbstractRenderer::Category::Objects);
  if (is_active()) {
    renderer.push_transformation(Viewport::default_transformation);

    QFont font = m_font_properties.get_font();
    QTextOption option = m_text_option_properties.get_option();
    const double width = property(WIDTH_PROPERTY_KEY).value<double>();
    const auto options = AbstractRenderer::TextOptions(font, option, style, width);
    renderer.draw_text(property(TEXT_PROPERTY_KEY).value<std::string>(), options);
    renderer.pop_transformation();
  }
}

}  // namespace omm
