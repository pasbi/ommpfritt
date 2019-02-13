#pragma once

#include <QWidget>
#include "abstractfactory.h"

class QComboBox;
class QLineEdit;
class QVBoxLayout;
class QFormLayout;

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

protected:
  QVBoxLayout* box_layout() const;
  QFormLayout* form_layout() const;

private:
  QComboBox* m_type_combobox;
  QLineEdit* m_name_edit;
  const std::vector<std::string> m_property_types;
  Property& m_property;
  void set_property_type(const std::string& type);

  QVBoxLayout* m_box_layout;
  QFormLayout* m_form_layout;
  QLayout* layout() const = delete;
};

template<typename PropertyT>
class PropertyConfigWidget : public AbstractPropertyConfigWidget
{
public:
  using property_type = PropertyT;
  static const std::string TYPE;
  using AbstractPropertyConfigWidget::AbstractPropertyConfigWidget;
};

template<typename PropertyT> const std::string
PropertyConfigWidget<PropertyT>::TYPE = std::string(PropertyT::TYPE) + "ConfigWidget";

void register_propertyconfigwidgets();

}  // namespace omm
