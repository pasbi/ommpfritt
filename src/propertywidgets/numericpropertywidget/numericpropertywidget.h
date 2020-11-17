#pragma once

#include "properties/floatproperty.h"
#include "properties/integerproperty.h"
#include "propertywidgets/multivalueedit.h"
#include "propertywidgets/numericpropertywidget/numericmultivalueedit.h"
#include "propertywidgets/propertywidget.h"

namespace omm
{
template<typename NumericPropertyT>
class NumericPropertyWidget : public PropertyWidget<NumericPropertyT>
{
public:
  using value_type = typename NumericPropertyT::value_type;
  explicit NumericPropertyWidget(Scene& scene, const std::set<Property*>& properties);

protected:
  void update_edit() override;
  void update_configuration() override;
  auto spinbox() const
  {
    return m_spinbox;
  }

private:
  NumericMultiValueEdit<value_type>* m_spinbox;
};

}  // namespace omm
