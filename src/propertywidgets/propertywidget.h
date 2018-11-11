#pragma once

#include <QWidget>
#include "properties/typedproperty.h"
#include "common.h"

namespace omm
{

class AbstractPropertyWidget
  : public QWidget
  , public AbstractFactory< std::string, AbstractPropertyWidget,
                            const std::set<Property*>& >
{
public:
  using SetOfProperties = std::set<Property*>;
  explicit AbstractPropertyWidget(const SetOfProperties& properties);
  virtual ~AbstractPropertyWidget();
  static void register_propertywidgets();

protected:
  virtual std::string label() const;
  void set_default_layout(std::unique_ptr<QWidget> other);
  std::unique_ptr<QWidget> make_label_widget() const;

private:
  const std::string m_label;
};

namespace detail
{

template<typename ValueT>
auto cast_all(const AbstractPropertyWidget::SetOfProperties& properties)
{
  return transform< omm::TypedProperty<ValueT>*,
                    omm::Property*,
                    std::set
                    >(properties, [](omm::Property* property) {
    return &property->cast<ValueT>();
  });
}

}

template<typename ValueT>
class PropertyWidget : public AbstractPropertyWidget, public AbstractTypedPropertyObserver
{
public:
  using property_type = TypedProperty<ValueT>;
  explicit PropertyWidget(const SetOfProperties& properties)
    : AbstractPropertyWidget(properties)
    , m_properties(detail::cast_all<ValueT>(properties))
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
    for (auto property : m_properties) {
      property->set_value(value);
    }
  }

  auto get_properties_values() const
  {
    return ::transform<ValueT>(m_properties, [](auto property) { return property->value(); });
  }

private:
  std::set<property_type*> m_properties;
};


}  // namespace omm
