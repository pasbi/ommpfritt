#pragma once

#include <QWidget>
#include "abstractfactory.h"

class QComboBox;

namespace omm
{

class AbstractPropertyConfigWidget
  : public QWidget
  , public AbstractFactory<std::string, AbstractPropertyConfigWidget, QWidget*>
{
public:
  explicit AbstractPropertyConfigWidget(QWidget* parent);
private:
  QComboBox* m_type_combobox;
  const std::vector<std::string> m_property_types;
};

template<typename PropertyT>
class PropertyConfigWidget : public AbstractPropertyConfigWidget
{
public:
  using property_type = PropertyT;
  static const std::string TYPE;
protected:
  using AbstractPropertyConfigWidget::AbstractPropertyConfigWidget;
};

template<typename PropertyT> const std::string
PropertyConfigWidget<PropertyT>::TYPE = std::string(PropertyT::TYPE) + "ConfigWidget";

void register_propertyconfigwidgets();

}  // namespace omm
