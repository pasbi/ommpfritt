#pragma once

#include <QComboBox>
#include "propertywidgets/multivalueedit.h"

namespace omm
{

class OptionsEdit : public QComboBox, public MultiValueEdit<size_t>
{
public:
  explicit OptionsEdit(const on_value_changed_t& on_value_changed);
  void set_options(const std::vector<std::string>& options);
  void set_value(const value_type& value) override;
  value_type value() const override;

protected:
  void set_inconsistent_value() override;
};

}  // namespace omm
