#pragma once

#include <QWidget>
#include "abstractfactory.h"

class QComboBox;
class QLineEdit;

namespace omm
{

class Property;
class AbstractPropertyConfigWidget
  : public QWidget
  , public AbstractFactory<std::string, AbstractPropertyConfigWidget, QWidget*, Property&>
{
  Q_OBJECT

public:
  explicit AbstractPropertyConfigWidget(QWidget* parent, Property& property);

Q_SIGNALS:
  void property_type_changed(const std::string& type);
  void property_label_changed();

private:
  QComboBox* m_type_combobox;
  QLineEdit* m_name_edit;
  const std::vector<std::string> m_property_types;
  Property& m_property;
  void set_property_type(const std::string& type);
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
