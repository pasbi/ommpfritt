#pragma once

#include "propertywidgets/propertywidget.h"

namespace omm
{

class IntegerPropertyWidget : public PropertyWidget<int>
{
public:
  using PropertyWidget<int>::PropertyWidget;
protected:
  void on_value_changed() override {}
  std::string type() const override;
};

}  // namespace omm
