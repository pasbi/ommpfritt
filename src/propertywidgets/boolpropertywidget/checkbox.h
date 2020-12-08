#pragma once

#include "propertywidgets/multivalueedit.h"
#include <QCheckBox>

namespace omm
{
class CheckBox
    : public QCheckBox
    , public MultiValueEdit<bool>
{
public:
  explicit CheckBox(const QString& label = "");
  void set_value(const value_type& value) override;
  [[nodiscard]] value_type value() const override;
  void nextCheckState() override;

protected:
  void set_inconsistent_value() override;
};

}  // namespace omm
