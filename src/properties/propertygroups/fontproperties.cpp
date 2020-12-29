#include "properties/propertygroups/fontproperties.h"
#include "aspects/propertyowner.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "properties/optionproperty.h"
#include "properties/stringproperty.h"

namespace omm
{
void FontProperties::make_properties(const QString& category) const
{
  static constexpr double DEFAULT_FONT_SIZE = 12.0;
  static constexpr double MIN_FONT_SIZE = 0.1;
  static constexpr double MAX_FONT_SIZE = 100.0;
  static constexpr int MAX_FONT_WEIGHT = 99;
  static constexpr double MAX_WORD_SPACING = 100.0;
  create_property<StringProperty>(FONT_PROPERTY_KEY, "Arial")
      .set_mode(StringProperty::Mode::Font)
      .set_label(QObject::tr("Font"))
      .set_category(category);
  create_property<FloatProperty>(SIZE_PROPERTY_KEY, DEFAULT_FONT_SIZE)
      .set_range(MIN_FONT_SIZE, MAX_FONT_SIZE)
      .set_label(QObject::tr("Size"))
      .set_category(category);
  create_property<IntegerProperty>(WEIGHT_PROPERTY_KEY)
      .set_range(0, MAX_FONT_WEIGHT)
      .set_label(QObject::tr("Weight"))
      .set_category(category);
  create_property<BoolProperty>(ITALIC_PROPERTY_KEY)
      .set_label(QObject::tr("Italic"))
      .set_category(category);
  create_property<BoolProperty>(UNDERLINE_PROPERTY_KEY)
      .set_label(QObject::tr("Underline"))
      .set_category(category);
  create_property<BoolProperty>(OVERLINE_PROPERTY_KEY)
      .set_label(QObject::tr("Overline"))
      .set_category(category);
  create_property<BoolProperty>(STRIKEOUT_PROPERTY_KEY)
      .set_label(QObject::tr("Strikeout"))
      .set_category(category);
  create_property<OptionProperty>(CAPITALIZATION_PROPERTY_KEY)
      .set_options({QObject::tr("Mixed"),
                    QObject::tr("All upper"),
                    QObject::tr("All lower"),
                    QObject::tr("Small caps"),
                    QObject::tr("Capitalize")})
      .set_label(QObject::tr("Case"))
      .set_category(category);
  create_property<BoolProperty>(FIXED_PITCH_PROPERTY_KEY)
      .set_label(QObject::tr("Fixed pitch"))
      .set_category(category);
  create_property<BoolProperty>(KERNING_PROPERTY_KEY)
      .set_label(QObject::tr("Kerning"))
      .set_category(category);
  create_property<OptionProperty>(LETTER_SPACING_TYPE_PROPERTY_KEY)
      .set_options({QObject::tr("Relative"), QObject::tr("Absolute")})
      .set_label(QObject::tr("Letter spacing mode"))
      .set_category(category);
  create_property<FloatProperty>(LETTER_SPACING_PROPERTY_KEY)
      .set_label(QObject::tr("Letter spacing"))
      .set_category(category);
  create_property<FloatProperty>(WORD_SPACING_PROPERTY_KEY)
      .set_range(0.0, MAX_WORD_SPACING)
      .set_label(QObject::tr("Word spacing"))
      .set_category(category);
}

QFont FontProperties::get_font() const
{
  QFont font;
  font.setFamily(property_value<QString>(FONT_PROPERTY_KEY));
  font.setStrikeOut(property_value<bool>(STRIKEOUT_PROPERTY_KEY));
  font.setUnderline(property_value<bool>(UNDERLINE_PROPERTY_KEY));
  font.setWeight(property_value<int>(WEIGHT_PROPERTY_KEY));
  font.setItalic(property_value<bool>(ITALIC_PROPERTY_KEY));
  font.setPointSizeF(property_value<double>(SIZE_PROPERTY_KEY));
  font.setCapitalization(property_value<QFont::Capitalization>(CAPITALIZATION_PROPERTY_KEY));
  font.setFixedPitch(property_value<bool>(FIXED_PITCH_PROPERTY_KEY));
  font.setKerning(property_value<bool>(KERNING_PROPERTY_KEY));
  font.setLetterSpacing(property_value<QFont::SpacingType>(LETTER_SPACING_TYPE_PROPERTY_KEY),
                        property_value<double>(LETTER_SPACING_PROPERTY_KEY));
  font.setOverline(property_value<bool>(OVERLINE_PROPERTY_KEY));
  font.setWordSpacing(property_value<double>(WORD_SPACING_PROPERTY_KEY));
  return font;
}

}  // namespace omm
