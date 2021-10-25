#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include <map>

class QCheckBox;

namespace omm
{

class ReferenceProperty;

class ReferencePropertyConfigWidget : public PropertyConfigWidget<ReferenceProperty>
{
  Q_OBJECT
public:
  ReferencePropertyConfigWidget();
  void init(const PropertyConfiguration& configuration) override;
  void update(PropertyConfiguration& configuration) const override;

private:
  std::map<Kind, QCheckBox*> m_allowed_kind_checkboxes;
  std::map<Flag, QCheckBox*> m_required_flag_checkboxes;
};

}  // namespace omm
