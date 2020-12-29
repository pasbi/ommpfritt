#include "properties/propertygroups/textoptionproperties.h"
#include "aspects/propertyowner.h"
#include "properties/boolproperty.h"
#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "properties/optionproperty.h"
#include "properties/stringproperty.h"

namespace omm
{
void TextOptionProperties::make_properties(const QString& category) const
{
  static constexpr double DEFAULT_TAB_STOP_WIDTH = 10.0;
  static constexpr double MAX_TAB_STOP_WIDTH = 100.0;
  create_property<OptionProperty>(ALIGNH_PROPERTY_KEY)
      .set_options({QObject::tr("Left"),
                    QObject::tr("Center"),
                    QObject::tr("Right"),
                    QObject::tr("Justify")})
      .set_label(QObject::tr("H-Align"))
      .set_category(category);
  create_property<OptionProperty>(ALIGNV_PROPERTY_KEY)
      .set_options({
          QObject::tr("Top"),
          QObject::tr("Center"),
          QObject::tr("Bottom")
          // the baseline flag cannot be supported right now because of QTBUG-74652:
          // https://bugreports.qt.io/browse/QTBUG-74652
          // https://stackoverflow.com/q/55296327/4248972
          //, QObject::tr("Baseline")
      })
      .set_label(QObject::tr("V-Align"))
      .set_category(category);
  create_property<FloatProperty>(TABSTOPDISTANCE_PROPERTY_KEY, DEFAULT_TAB_STOP_WIDTH)
      .set_range(0, MAX_TAB_STOP_WIDTH)
      .set_label(QObject::tr("Tab width"))
      .set_category(category);
  create_property<OptionProperty>(DIRECTION_PROPERTY_KEY)
      .set_options(
          {QObject::tr("Left to right"), QObject::tr("Right to left"), QObject::tr("Auto")})
      .set_label(QObject::tr("Direction"))
      .set_category(category);
  create_property<OptionProperty>(WRAP_MODE_PROPERTY_KEY)
      .set_options({QObject::tr("none"),
                    QObject::tr("at word boundary"),
                    QObject::tr("anywhere"),
                    QObject::tr("prefer at word boundary")})
      .set_label(QObject::tr("Wrap"))
      .set_category(category);
}

QTextOption TextOptionProperties::get_option() const
{
  QTextOption option;
  Qt::Alignment alignment = [f = property_value<std::size_t>(ALIGNH_PROPERTY_KEY)]() {
    switch (f) {
    case 0:
      return Qt::AlignLeft;
    case 1:
      return Qt::AlignHCenter;
    case 2:
      return Qt::AlignRight;
    case 3:
      return Qt::AlignJustify;
    default:
      LFATAL("Unexpected case.");
      return Qt::AlignCenter;
    }
  }();
  alignment |= [f = property_value<std::size_t>(ALIGNV_PROPERTY_KEY)]() {
    switch (f) {
    case 0:
      return Qt::AlignTop;
    case 1:
      return Qt::AlignVCenter;
    case 2:
      return Qt::AlignBottom;
    case 3:
      return Qt::AlignBaseline;  // is never reached (see @code make_properties)
    default:
      LFATAL("Unexpected case.");
      return Qt::AlignCenter;
    }
  }();
  option.setAlignment(alignment);

  // Don't stumble into the pitfall! // https://bugreports.qt.io/browse/QTBUG-74652
  assert(option.alignment() == alignment);

  option.setTabStopDistance(property_value<double>(TABSTOPDISTANCE_PROPERTY_KEY));
  option.setTextDirection(property_value<Qt::LayoutDirection>(DIRECTION_PROPERTY_KEY));

  const auto wrap_mode = [w = property_value<std::size_t>(WRAP_MODE_PROPERTY_KEY)]() {
    switch (w) {
    case 0:
      return QTextOption::NoWrap;
    case 1:
      return QTextOption::WordWrap;
    case 2:
      return QTextOption::WrapAnywhere;
    case 3:
      return QTextOption::WrapAtWordBoundaryOrAnywhere;
    default:
      LFATAL("Unexpected case.");
      return QTextOption::NoWrap;
    }
  }();
  option.setWrapMode(wrap_mode);
  return option;
}

}  // namespace omm
