#pragma once

#include <QLineEdit>
#include "propertywidgets/multivalueedit.h"

namespace omm
{

class LineEdit : public QLineEdit, public MultiValueEdit<std::string>
{
public:
  explicit LineEdit();
  void set_value(const value_type& value) override;
  value_type value() const override;

protected:
  void set_inconsistent_value() override;
};

}  // namespace omm
