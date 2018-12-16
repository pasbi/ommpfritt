#pragma once

#include <string>
#include "propertywidgets/propertywidget.h"
#include "properties/stringproperty.h"

namespace omm
{

class LineEdit;

class StringPropertyWidget : public PropertyWidget<StringProperty>
{
public:
  explicit StringPropertyWidget(Scene& scene, const Property::SetOfProperties& properties);

protected:
  void update_edit() override;
  std::string type() const override;

private:
  LineEdit* m_line_edit;
};

}  // namespace omm
