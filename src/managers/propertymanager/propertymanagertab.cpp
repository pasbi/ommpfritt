#include "managers/propertymanager/propertymanagertab.h"

#include <memory>
#include <QFormLayout>
#include <QLabel>
#include "properties/typedproperty.h"
#include "propertywidgets/propertywidget.h"

namespace omm
{

PropertyManagerTab::PropertyManagerTab()
{
  setWidgetResizable(true);
  auto layout = std::make_unique<QFormLayout>();
  m_layout = layout.get();
  m_widget = std::make_unique<QWidget>();
  m_widget->setLayout(layout.release());
}

PropertyManagerTab::~PropertyManagerTab()
{
}

void PropertyManagerTab::add_properties(Scene& scene, const std::string& key,
                                        const std::set<Property*>& properties)
{
  assert(properties.size() > 0);

  const auto label = Property::get_value<std::string>(properties, std::mem_fn(&Property::label));
  auto label_widget = std::make_unique<QLabel>();
  label_widget->setText(QString::fromStdString(label));
  label_widget->setToolTip(QString::fromStdString(key));

  const auto widget_type = (*properties.begin())->widget_type();
  auto property_widget = AbstractPropertyWidget::make(widget_type, scene, properties);

  label_widget->setBuddy(property_widget.get());
  m_layout->addRow(label_widget.release(), property_widget.release());
}

void PropertyManagerTab::end_add_properties()
{
  setWidget(m_widget.release());
}

}  // namespace omm
