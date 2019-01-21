#include "propertywidgets/propertywidget.h"

#include <QBoxLayout>
#include <QLabel>

namespace omm
{

AbstractPropertyWidget
::AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties)
  : m_label(Property::get_value<std::string>(properties, std::mem_fn(&Property::label)))
  , scene(scene)
{

}

void AbstractPropertyWidget::set_default_layout(std::unique_ptr<QWidget> other)
{
  auto layout = std::make_unique<QHBoxLayout>();
  layout->addWidget(make_label_widget().release(), 0);
  layout->addWidget(other.release(), 1);
  setLayout(layout.release());
}

std::unique_ptr<QWidget> AbstractPropertyWidget::make_label_widget() const
{
  return std::make_unique<QLabel>(QString::fromStdString(label()));
}

std::string AbstractPropertyWidget::label() const
{
  return m_label;
}

void AbstractPropertyWidget::on_property_value_changed(Property& property)
{
  update_edit();
}

}  // namespace omm
