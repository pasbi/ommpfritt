#include "managers/propertymanager/propertymanagertab.h"

#include <memory>
#include <QLayout>
#include <glog/logging.h>
#include "properties/property.h"
#include "propertywidgets/integerpropertywidget.h"
#include "propertywidgets/stringpropertywidget.h"
#include "propertywidgets/objecttransformationpropertywidget.h"

namespace
{

// todo typedef `std::unordered_set<omm::Property*>`

template<typename PropertyWidgetT>
std::unique_ptr<QWidget> make_property_widget(const std::unordered_set<omm::Property*>& properties)
{
  return std::make_unique<PropertyWidgetT>(properties);
}

template<typename PropertyWidgetT> auto make_map_item()
{
  return std::make_pair( PropertyWidgetT::property_type::TYPE_INDEX,
                         &make_property_widget<PropertyWidgetT>      );
}

std::type_index get_property_index(const std::unordered_set<omm::Property*>& properties)
{
  assert(properties.size() > 0);
  const auto type_index = (*properties.begin())->type_index();
#ifndef NDEBUG
  for (auto&& property : properties) {
    assert(type_index == property->type_index());
  }
#endif
  return type_index;
}

std::unique_ptr<QWidget> make_property_widget(const std::unordered_set<omm::Property*>& properties)
{
  using creator_type = std::unique_ptr<QWidget>(*)(const std::unordered_set<omm::Property*>&);
  using creator_map_type = std::map<std::type_index, creator_type>;
  static const creator_map_type widget_creators = {
    make_map_item<omm::IntegerPropertyWidget>(),
    make_map_item<omm::StringPropertyWidget>(),
    make_map_item<omm::ObjectTransformationPropertyWidget>()
  };

  return widget_creators.at(get_property_index(properties))(properties);
}

}  // namespace

namespace omm
{

PropertyManagerTab::PropertyManagerTab()
{
  setLayout(new QVBoxLayout());
}

void PropertyManagerTab::add_properties(const std::unordered_set<Property*>& properties)
{
  layout()->addWidget(make_property_widget(properties).release());
}

}  // namespace omm
