#pragma once

#include "properties/stringproperty.h"
#include "propertywidgets/propertywidget.h"
#include <string>

namespace omm
{
class AbstractTextEditAdapter;

class StringPropertyWidget : public PropertyWidget<StringProperty>
{
public:
  explicit StringPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;
  void update_configuration() override;

private:
  AbstractTextEditAdapter* m_text_edit;
};

}  // namespace omm
