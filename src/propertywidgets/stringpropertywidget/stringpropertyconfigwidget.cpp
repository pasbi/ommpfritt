#include "propertywidgets/stringpropertywidget/stringpropertyconfigwidget.h"
#include <QComboBox>
#include <QLayout>

namespace omm
{
StringPropertyConfigWidget::StringPropertyConfigWidget()
{
  auto layout = std::make_unique<QVBoxLayout>();
  auto mode_selector = std::make_unique<QComboBox>(this);
  mode_selector->addItems({QObject::tr("single line", "StringPropertyConfigWidget"),
                           QObject::tr("multi line", "StringPropertyConfigWidget"),
                           QObject::tr("file path", "StringPropertyConfigWidget"),
                           QObject::tr("code", "StringPropertyConfigWidget"),
                           QObject::tr("font", "StringPropertyConfigWidget")});

  m_mode_selector = mode_selector.get();
  layout->addWidget(mode_selector.release());
  layout->addStretch();
  setLayout(layout.release());
}

void StringPropertyConfigWidget::init(const PropertyConfiguration& configuration)
{
  m_mode_selector->setCurrentIndex(configuration.get("mode", std::size_t(3)));
}

void StringPropertyConfigWidget::update(PropertyConfiguration& configuration) const
{
  configuration.set("mode", static_cast<std::size_t>(m_mode_selector->currentIndex()));
}

}  // namespace omm
