#pragma once

#include <string>
#include "propertywidgets/propertywidget.h"
#include "properties/stringproperty.h"

namespace omm
{

class AbstractTextEditAdapter;

class StringPropertyWidget : public PropertyWidget<StringProperty>
{
public:
  explicit StringPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;
  std::string type() const override;

private:
  AbstractTextEditAdapter* m_text_edit;
};

}  // namespace omm
