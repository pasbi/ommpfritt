#include "managers/propertymanager/propertymanagertab.h"

#include <memory>
#include <QLayout>
#include <glog/logging.h>
#include "properties/typedproperty.h"
#include "propertywidgets/numericpropertywidget.h"
#include "propertywidgets/stringpropertywidget.h"
#include "propertywidgets/objecttransformationpropertywidget.h"

namespace
{

std::string get_property_index(const std::set<omm::Property*>& properties)
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
  auto layout = std::make_unique<QVBoxLayout>();
  m_layout = layout.get();
  setLayout(layout.release());
}

void PropertyManagerTab::add_properties(const AbstractPropertyWidget::SetOfProperties& properties)
{
  assert(properties.size() > 0);
  const auto widget_type = (*properties.begin())->widget_type();
  auto widget = AbstractPropertyWidget::make(widget_type, properties);
  m_layout->addWidget(widget.release());
}

void PropertyManagerTab::end_add_properties()
{
  m_layout->addStretch();
}

}  // namespace omm
