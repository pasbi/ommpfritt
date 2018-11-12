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
{
public:
  explicit AbstractPropertyWidget(Scene& scene, const Property::SetOfProperties& properties);
  virtual ~AbstractPropertyWidget();
  static void register_propertywidgets();

protected:
  virtual std::string label() const;
  void set_default_layout(std::unique_ptr<QWidget> other);
  std::unique_ptr<QWidget> make_label_widget() const;
  Scene& scene() const { return m_scene; }

private:
  const std::string m_label;
  Scene& m_scene;
};

template<typename ValueT>
class PropertyWidget : public AbstractPropertyWidget, public AbstractTypedPropertyObserver
{
public:
  using property_type = TypedProperty<ValueT>;
  using value_type = typename property_type::value_type;
  explicit PropertyWidget(Scene& scene, const Property::SetOfProperties& properties)
    : AbstractPropertyWidget(scene, properties)
    , m_properties(Property::cast_all<ValueT>(properties))
  {
    for (auto&& property : m_properties) {
      property->ObserverRegister<AbstractTypedPropertyObserver>::register_observer(*this);
    }
  }

  virtual ~PropertyWidget()
  {
    for (auto&& property : m_properties) {
      property->ObserverRegister<AbstractTypedPropertyObserver>::unregister_observer(*this);
    }
  }

protected:
  void set_properties_value(const ValueT& value)
  {
    scene().submit<PropertiesCommand<ValueT>>(m_properties, value);
  }

  auto get_properties_values() const
  {
    return ::transform<ValueT>(m_properties, [](auto property) { return property->value(); });
  }

private:
  std::set<property_type*> m_properties;
};


}  // namespace omm
