#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/optionsproperty.h"

class QListWidget;

namespace omm
{

class OptionsPropertyConfigWidget : public PropertyConfigWidget<OptionsProperty>
{
public:
  OptionsPropertyConfigWidget();
  void init(const Property::Configuration &configuration) override;
  void update(Property::Configuration &configuration) const override;

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  QListWidget* m_list_widget;

  void add_option(const QString& label);
  void remove_option(int index);
};

}  // namespace omm
