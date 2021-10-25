#pragma once

#include "properties/propertygroups/propertygroup.h"
#include <QFont>
#include <string>

namespace omm
{
class FontProperties : public PropertyGroup
{
public:
  using PropertyGroup::PropertyGroup;
  void make_properties(const QString& category) const override;

  QFont get_font() const;

  static constexpr auto FONT_PROPERTY_KEY = "font";
  static constexpr auto STRIKEOUT_PROPERTY_KEY = "strikeout";
  static constexpr auto UNDERLINE_PROPERTY_KEY = "underline";
  static constexpr auto WEIGHT_PROPERTY_KEY = "bold";
  static constexpr auto ITALIC_PROPERTY_KEY = "italic";
  static constexpr auto SIZE_PROPERTY_KEY = "size";
  static constexpr auto CAPITALIZATION_PROPERTY_KEY = "capitalization";
  static constexpr auto FIXED_PITCH_PROPERTY_KEY = "fixedpitch";
  static constexpr auto KERNING_PROPERTY_KEY = "kerning";
  static constexpr auto LETTER_SPACING_TYPE_PROPERTY_KEY = "letter_spacing_type";
  static constexpr auto LETTER_SPACING_PROPERTY_KEY = "letter_spacing";
  static constexpr auto OVERLINE_PROPERTY_KEY = "overline";
  static constexpr auto WORD_SPACING_PROPERTY_KEY = "word_spacing";
};

}  // namespace omm
