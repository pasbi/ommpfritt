#include "managers/propertymanager/userpropertymanager/propertyconfigwidget.h"

#include <QVBoxLayout>
#include <QComboBox>

namespace
{

template<typename Ts> auto to_vector(const Ts& ts)
{
  const auto f = [](auto&& s) { return std::forward<decltype(s)>(s); };
  return ::transform<std::string, std::vector>(ts, f);
}

}  // namespace

namespace omm
{

AbstractPropertyConfigWidget::AbstractPropertyConfigWidget(QWidget* parent)
  : QWidget(parent)
  , m_property_types(to_vector(AbstractPropertyConfigWidget::keys()))
{
  setLayout(std::make_unique<QVBoxLayout>(this).release());
  m_type_combobox = std::make_unique<QComboBox>(this).release();
  layout()->addWidget(m_type_combobox);

  m_type_combobox->addItems(::transform<QString, QList>(m_property_types, QString::fromStdString));
}

}  // namespace omm
