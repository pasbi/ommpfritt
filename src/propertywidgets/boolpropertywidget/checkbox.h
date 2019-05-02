#pragma once

#include <QCheckBox>
#include "propertywidgets/multivalueedit.h"

namespace omm
{

class CheckBox : public QCheckBox, public MultiValueEdit<bool>
{
public:
  explicit CheckBox();
  void set_value(const value_type& value) override;
  value_type value() const override;
  void nextCheckState() override;

protected:
  void set_inconsistent_value() override;
};

}  // namespace omm
