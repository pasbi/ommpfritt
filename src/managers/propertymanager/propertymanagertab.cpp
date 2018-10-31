#include "managers/propertymanager/propertymanagertab.h"

#include <memory>
#include <QLayout>
#include <QPushButton> // TODO remove
#include "properties/property.h"

namespace
{

std::unique_ptr<QWidget> make_property_widget(omm::Property* property)
{
  return std::make_unique<QPushButton>(QString::fromStdString(property->label()));
}

}  // namespace

namespace omm
{

PropertyManagerTab::PropertyManagerTab()
{
  setLayout(new QVBoxLayout());
}

void PropertyManagerTab::add_properties(const std::vector<Property*>& properties)
{
  for (Property* property : properties) {
    layout()->addWidget(make_property_widget(property).release());
  }
}

}  // namespace omm
