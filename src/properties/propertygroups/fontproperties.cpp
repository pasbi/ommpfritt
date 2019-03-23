#include "properties/propertygroups/fontproperties.h"
#include "aspects/propertyowner.h"
#include "properties/floatproperty.h"
#include "properties/stringproperty.h"
#include "properties/optionsproperty.h"
#include "properties/boolproperty.h"
#include <QDebug>
#include <properties/integerproperty.h>

namespace omm
{

void FontProperties::make_properties(const std::string& category)
{
  add_property<StringProperty>(key(FONT_PROPERTY_KEY), "Arial")
    .set_mode(StringProperty::Mode::Font)
    .set_label(QObject::tr("Font").toStdString()).set_category(category);
  add_property<FloatProperty>(key(SIZE_PROPERTY_KEY), 12)
    .set_range(0.1, 100)
    .set_label(QObject::tr("Size").toStdString()).set_category(category);
  add_property<IntegerProperty>(key(WEIGHT_PROPERTY_KEY))
    .set_range(0, 99)
    .set_label(QObject::tr("Weight").toStdString()).set_category(category);
  add_property<BoolProperty>(key(ITALIC_PROPERTY_KEY))
    .set_label(QObject::tr("Italic").toStdString()).set_category(category);
  add_property<BoolProperty>(key(UNDERLINE_PROPERTY_KEY))
    .set_label(QObject::tr("Underline").toStdString()).set_category(category);
  add_property<BoolProperty>(key(OVERLINE_PROPERTY_KEY))
    .set_label(QObject::tr("Overline").toStdString()).set_category(category);
  add_property<BoolProperty>(key(STRIKEOUT_PROPERTY_KEY))
    .set_label(QObject::tr("Strikeout").toStdString()).set_category(category);
  add_property<OptionsProperty>(key(CAPITALIZATION_PROPERTY_KEY))
    .set_options({ QObject::tr("Mixed").toStdString(),
                   QObject::tr("All upper").toStdString(),
                   QObject::tr("All lower").toStdString(),
                   QObject::tr("Small caps").toStdString(),
                   QObject::tr("Capitalize").toStdString() })
    .set_label(QObject::tr("Case").toStdString()).set_category(category);
  add_property<BoolProperty>(key(FIXED_PITCH_PROPERTY_KEY))
    .set_label(QObject::tr("Fixed pitch").toStdString()).set_category(category);
  add_property<BoolProperty>(key(KERNING_PROPERTY_KEY))
    .set_label(QObject::tr("Kerning").toStdString()).set_category(category);
  add_property<OptionsProperty>(key(LETTER_SPACING_TYPE_PROPERTY_KEY))
    .set_options({ QObject::tr("Relative").toStdString(), QObject::tr("Absolute").toStdString() })
    .set_label(QObject::tr("Letter spacing mode").toStdString()).set_category(category);
  add_property<FloatProperty>(key(LETTER_SPACING_PROPERTY_KEY))
    .set_label(QObject::tr("Letter spacing").toStdString()).set_category(category);
  add_property<FloatProperty>(key(WORD_SPACING_PROPERTY_KEY))
    .set_range(0.0, 100.0)
    .set_label(QObject::tr("Word spacing").toStdString()).set_category(category);
}

QFont FontProperties::get_font() const
{
  QFont font;
  font.setFamily(property_value<std::string>(FONT_PROPERTY_KEY).c_str());
  font.setStrikeOut(property_value<bool>(STRIKEOUT_PROPERTY_KEY));
  font.setUnderline(property_value<bool>(UNDERLINE_PROPERTY_KEY));
  font.setWeight(property_value<int>(WEIGHT_PROPERTY_KEY));
  font.setItalic(property_value<bool>(ITALIC_PROPERTY_KEY));
  font.setPointSizeF(property_value<double>(SIZE_PROPERTY_KEY));
  font.setCapitalization(property_value<QFont::Capitalization>(CAPITALIZATION_PROPERTY_KEY));
  font.setFixedPitch(property_value<bool>(FIXED_PITCH_PROPERTY_KEY));
  font.setKerning(property_value<bool>(KERNING_PROPERTY_KEY));
  font.setLetterSpacing( property_value<QFont::SpacingType>(LETTER_SPACING_TYPE_PROPERTY_KEY),
                         property_value<double>(LETTER_SPACING_PROPERTY_KEY) );
  font.setOverline(property_value<bool>(OVERLINE_PROPERTY_KEY));
  font.setWordSpacing(property_value<double>(WORD_SPACING_PROPERTY_KEY));
  return font;
}


}  // namespace omm
