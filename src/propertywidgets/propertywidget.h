#pragma once

#include <QWidget>
#include "properties/property.h"
#include "common.h"

namespace omm
{

class AbstractPropertyWidget : public QWidget
{
public:
  using SetOfProperties = std::unordered_set<Property*>;
  virtual ~AbstractPropertyWidget();
protected:
  virtual std::string label() const = 0;
  void set_default_layout(std::unique_ptr<QWidget> other);
  std::unique_ptr<QWidget> make_label_widget() const;
};

namespace detail
{

template<typename ValueT>
auto cast_all(const AbstractPropertyWidget::SetOfProperties& properties)
{
  return transform< omm::TypedProperty<ValueT>*,
                    omm::Property*,
                    std::unordered_set
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
    : m_properties(detail::cast_all<ValueT>(properties))
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

  std::string label() const override { return "XXX"; } // m_property.label(); }

protected:
  std::unordered_set<property_type*> m_properties;
};

}  // namespace omm
