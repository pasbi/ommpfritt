#include "managers/propertymanager/propertymanagertab.h"

#include <memory>
#include <QLayout>
#include <glog/logging.h>
#include "properties/typedproperty.h"
#include "propertywidgets/integerpropertywidget.h"
#include "propertywidgets/stringpropertywidget.h"
#include "propertywidgets/objecttransformationpropertywidget.h"

namespace
{

std::string get_property_index(const std::unordered_set<omm::Property*>& properties)
{
  assert(properties.size() > 0);
  const auto type = (*properties.begin())->type();
#ifndef NDEBUG
  for (auto&& property : properties) {
    assert(type == property->type());
  }
#endif
  return type;
}

}  // namespace

namespace omm
{

PropertyManagerTab::PropertyManagerTab()
{
  setLayout(new QVBoxLayout());
}

void PropertyManagerTab::add_properties(const AbstractPropertyWidget::SetOfProperties& properties)
{
  assert(properties.size() > 0);
  const auto widget_type = (*properties.begin())->widget_type();
  auto widget = AbstractPropertyWidget::make(widget_type, properties);
  layout()->addWidget(widget.release());
}

}  // namespace omm
