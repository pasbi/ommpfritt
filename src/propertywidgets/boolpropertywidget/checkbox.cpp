#include "propertywidgets/boolpropertywidget/checkbox.h"

namespace omm
{
CheckBox::CheckBox(const QString& label) : QCheckBox(label)
{
  setTristate(true);
}
void CheckBox::set_value(const value_type& value)
{
  setChecked(value);
}
void CheckBox::set_inconsistent_value()
{
  setCheckState(Qt::PartiallyChecked);
}
CheckBox::value_type CheckBox::value() const
{
  return isChecked();
}
void CheckBox::nextCheckState()
{
  setChecked(!isChecked());
}

}  // namespace omm
