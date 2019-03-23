#include "properties/propertygroups/textoptionproperties.h"
#include "aspects/propertyowner.h"
#include "properties/floatproperty.h"
#include "properties/stringproperty.h"
#include "properties/optionsproperty.h"
#include "properties/boolproperty.h"
#include <QDebug>
#include <properties/integerproperty.h>

namespace omm
{

void TextOptionProperties::make_properties(const std::string& category)
{
  add_property<OptionsProperty>(key(ALIGNH_PROPERTY_KEY))
    .set_options({ QObject::tr("Left").toStdString(), QObject::tr("Center").toStdString(),
                   QObject::tr("Right").toStdString(), QObject::tr("Justify").toStdString() })
    .set_label(QObject::tr("H-Align").toStdString()).set_category(category);
  add_property<OptionsProperty>(key(ALIGNV_PROPERTY_KEY))
    .set_options({ QObject::tr("Top").toStdString(), QObject::tr("Center").toStdString(),
                   QObject::tr("Bottom").toStdString()
                   // the baseline flag cannot be supported right now because of QTBUG-74652:
                   // https://bugreports.qt.io/browse/QTBUG-74652
                   // https://stackoverflow.com/q/55296327/4248972
                   //, QObject::tr("Baseline").toStdString()
                 })
    .set_label(QObject::tr("V-Align").toStdString()).set_category(category);
  add_property<FloatProperty>(key(TABSTOPDISTANCE_PROPERTY_KEY), 10)
    .set_range(0, 100)
    .set_label(QObject::tr("Tab width").toStdString()).set_category(category);
  add_property<OptionsProperty>(key(DIRECTION_PROPERTY_KEY))
      .set_options({ QObject::tr("Left to right").toStdString(),
                     QObject::tr("Right to left").toStdString(),
                     QObject::tr("Auto").toStdString() })
      .set_label(QObject::tr("Direction").toStdString()).set_category(category);
  add_property<OptionsProperty>(key(WRAP_MODE_PROPERTY_KEY))
      .set_options({ QObject::tr("none").toStdString(),
                     QObject::tr("at word boundary").toStdString(),
                     QObject::tr("anywhere").toStdString(),
                     QObject::tr("prefer at word boundary").toStdString() })
      .set_label(QObject::tr("Wrap").toStdString()).set_category(category);
}

QTextOption TextOptionProperties::get_option() const
{
  QTextOption option;
  Qt::Alignment alignment = [f=property_value<std::size_t>(ALIGNH_PROPERTY_KEY)]() {
    switch (f) {
    case 0: return Qt::AlignLeft;
    case 1: return Qt::AlignHCenter;
    case 2: return Qt::AlignRight;
    case 3: return Qt::AlignJustify;
    default: qFatal("Unexpected case.");
    }
  }();
  alignment |= [f=property_value<std::size_t>(ALIGNV_PROPERTY_KEY)]() {
    switch (f) {
    case 0: return Qt::AlignTop;
    case 1: return Qt::AlignVCenter;
    case 2: return Qt::AlignBottom;
    case 3: return Qt::AlignBaseline;   // is never reached (see @code make_properties)
    default: qFatal("Unexpected case.");
    }
  }();
  option.setAlignment(alignment);

  // Don't stumble into the pitfall! // https://bugreports.qt.io/browse/QTBUG-74652
  assert(option.alignment() == alignment);

  option.setTabStopDistance(property_value<double>(TABSTOPDISTANCE_PROPERTY_KEY));
  option.setTextDirection(property_value<Qt::LayoutDirection>(DIRECTION_PROPERTY_KEY));

  const auto wrap_mode = [w=property_value<std::size_t>(WRAP_MODE_PROPERTY_KEY)]() {
    switch (w) {
    case 0: return QTextOption::NoWrap;
    case 1: return QTextOption::WordWrap;
    case 2: return QTextOption::WrapAnywhere;
    case 3: return QTextOption::WrapAtWordBoundaryOrAnywhere;
    default: qFatal("Unexpected case.");
    }
  }();
  option.setWrapMode(wrap_mode);
  return option;
}

}  // namespace omm
