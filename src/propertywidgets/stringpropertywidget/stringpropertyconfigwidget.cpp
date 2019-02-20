#include "propertywidgets/stringpropertywidget/stringpropertyconfigwidget.h"
#include <QComboBox>
#include <QLayout>

namespace omm
{

StringPropertyConfigWidget::StringPropertyConfigWidget(QWidget* parent, Property& property)
  : PropertyConfigWidget<StringProperty>(parent, property)
{
  auto& string_property = static_cast<StringProperty&>(property);

  auto* mode_selector = std::make_unique<QComboBox>(this).release();
  mode_selector->addItems({ tr("single line"), tr("multi line"), tr("file path"), tr("code") });
  box_layout()->addWidget(mode_selector);
  mode_selector->setCurrentIndex(static_cast<int>(string_property.mode()));
  const auto cic = static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged);
  connect(mode_selector, cic, [&string_property](int index) {
    string_property.set_mode(static_cast<StringProperty::Mode>(index));
  });
  box_layout()->addStretch();
}

std::string StringPropertyConfigWidget::type() const
{
  return TYPE;
}

}  // namespace pmm

