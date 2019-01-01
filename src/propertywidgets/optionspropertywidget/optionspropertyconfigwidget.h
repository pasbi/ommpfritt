#pragma once

#include "propertywidgets/propertyconfigwidget.h"
#include "properties/optionsproperty.h"

class QListWidget;

namespace omm
{

class OptionsPropertyConfigWidget : public PropertyConfigWidget<OptionsProperty>
{
public:
  OptionsPropertyConfigWidget(QWidget* parent, Property& property);
  std::string type() const override;

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void update_property_options();
  QListWidget* m_list_widget;
  OptionsProperty& m_options_property;

  void add_option(const std::string& label);
  void remove_option(int index);
};

}  // namespace omm
