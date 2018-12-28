#include "managers/propertymanager/userpropertymanager/stringpropertyconfigwidget.h"
#include <QCheckBox>
#include <QLayout>

namespace omm
{

StringPropertyConfigWidget::StringPropertyConfigWidget(QWidget* parent, Property& property)
  : PropertyConfigWidget<StringProperty>(parent, property)
{
  auto& string_property = static_cast<StringProperty&>(property);
  auto* check_box = std::make_unique<QCheckBox>("multi line", this).release();
  layout()->addWidget(check_box);
  check_box->setChecked(string_property.is_multi_line());
  connect(check_box, &QCheckBox::clicked, [&string_property](bool checked) {
    string_property.set_is_multi_line(checked);
  });
}

std::string StringPropertyConfigWidget::type() const
{
  return TYPE;
}

}  // namespace pmm

