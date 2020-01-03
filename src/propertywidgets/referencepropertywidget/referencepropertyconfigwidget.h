#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/referenceproperty.h"
#include "aspects/propertyowner.h"
#include <map>

class QCheckBox;

namespace omm
{

class ReferencePropertyConfigWidget : public PropertyConfigWidget
{
  Q_OBJECT
public:
  ReferencePropertyConfigWidget();
  static constexpr auto TYPE = "ReferencePropertyConfigWidget";
  QString type() const override { return TYPE; }
  void init(const Property::Configuration &configuration) override;
  void update(Property::Configuration &configuration) const override;

private:
  std::map<Kind, QCheckBox*> m_allowed_kind_checkboxes;
  std::map<Flag, QCheckBox*> m_required_flag_checkboxes;

};

}  // namespace omm
