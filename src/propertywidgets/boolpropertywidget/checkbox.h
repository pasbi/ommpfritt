#pragma once

#include <QCheckBox>
#include "propertywidgets/multivalueedit.h"

namespace omm
{

class CheckBox : public QCheckBox, public MultiValueEdit<bool>
{
public:
  explicit CheckBox(const on_value_changed_t& on_value_changed);
  void set_value(const value_type& value) override;
  value_type value() const override;
  void nextCheckState() override;

protected:
  void set_inconsistent_value() override;
};

}  // namespace omm
