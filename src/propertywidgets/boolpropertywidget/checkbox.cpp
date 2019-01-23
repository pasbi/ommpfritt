#include "propertywidgets/boolpropertywidget/checkbox.h"

namespace omm
{

CheckBox::CheckBox(const on_value_changed_t& on_value_changed)
  : MultiValueEdit<bool>(on_value_changed)
{
  setTristate(true);
  connect(this, &QCheckBox::toggled, [=](const bool checked) { on_value_changed(checked); });
}

void CheckBox::set_value(const value_type& value) { setChecked(value); }
void CheckBox::set_inconsistent_value() { setCheckState(Qt::PartiallyChecked); }
CheckBox::value_type CheckBox::value() const { return isChecked(); }
void CheckBox::nextCheckState() { setChecked(!isChecked()); }

}  // namespace omm
