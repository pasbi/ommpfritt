#pragma once

#include "propertywidgets/propertyconfigwidget.h"

class QListWidget;

namespace omm
{

class OptionProperty;

class OptionPropertyConfigWidget : public PropertyConfigWidget<OptionProperty>
{
public:
  OptionPropertyConfigWidget();
  void init(const PropertyConfiguration& configuration) override;
  void update(PropertyConfiguration& configuration) const override;

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  QListWidget* m_list_widget;

  void add_option(const QString& label);
  void remove_option(int index);
};

}  // namespace omm
