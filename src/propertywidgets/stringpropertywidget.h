#pragma once

#include <string>
#include "propertywidgets/propertywidget.h"

class QLineEdit;

namespace omm
{

class StringPropertyWidget : public PropertyWidget<std::string>
{
public:
  explicit StringPropertyWidget(const SetOfProperties& properties);
protected:
  void on_value_changed() override;
  std::string type() const override;

private:
  class LineEdit;
  LineEdit* m_line_edit;
};

}  // namespace omm
