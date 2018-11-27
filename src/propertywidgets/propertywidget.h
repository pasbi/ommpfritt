#pragma once

#include <QWidget>
#include "properties/typedproperty.h"
#include "common.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"

namespace omm
{

class AbstractPropertyWidget
  : public QWidget
  , public AbstractFactory< std::string, AbstractPropertyWidget,
                            Scene&, const Property::SetOfProperties& >
  , public AbstractPropertyObserver
{
public:
  explicit AbstractPropertyWidget(Scene& scene, const Property::SetOfProperties& properties);
  virtual ~AbstractPropertyWidget();

protected:
  virtual std::string label() const;
  void set_default_layout(std::unique_ptr<QWidget> other);
  std::unique_ptr<QWidget> make_label_widget() const;
  Scene& scene() const { return m_scene; }

private:
  const std::string m_label;
  Scene& m_scene;
};

template<typename PropertyT>
class PropertyWidget : public AbstractPropertyWidget
{
public:
  using property_type = PropertyT;
  using value_type = typename property_type::value_type;
  explicit PropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
    : AbstractPropertyWidget(scene, properties)
    , m_properties(Property::cast_all<property_type>(properties))
  {
    for (auto&& property : m_properties) {
      property->Observed<AbstractPropertyObserver>::register_observer(*this);
    }
  }

  virtual ~PropertyWidget()
  {
    for (auto&& property : m_properties) {
      property->Observed<AbstractPropertyObserver>::unregister_observer(*this);
    }
  }

protected:
  void set_properties_value(const value_type& value)
  {
    scene().submit<PropertiesCommand<property_type>>(m_properties, value);
  }

  auto get_properties_values() const
  {
    return ::transform<value_type>(m_properties, [](auto property) { return property->value(); });
  }

  const std::set<property_type*>& properties() const { return m_properties; }

private:
  std::set<property_type*> m_properties;
};

void register_propertywidgets();

}  // namespace omm
