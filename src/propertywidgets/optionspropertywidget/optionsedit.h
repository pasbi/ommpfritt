#pragma once

#include <QComboBox>
#include "propertywidgets/multivalueedit.h"

namespace omm
{

class OptionsEdit : public QComboBox, public MultiValueEdit<size_t>
{
public:
  void set_options(const std::vector<QString>& options);
  void set_value(const value_type& value) override;
  value_type value() const override;

protected:
  void set_inconsistent_value() override;
};

}  // namespace omm
