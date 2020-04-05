#pragma once
#include <QWidget>
#include "properties/typedproperty.h"
#include "common.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"
#include <QTimer>
#include <QBoxLayout>
#include "scene/history/macro.h"
#include <QLabel>

namespace omm
{

class AbstractPropertyWidget
  : public QWidget
  , public AbstractFactory< QString, false, AbstractPropertyWidget,
                            Scene&, const std::set<Property*>& >
{
  Q_OBJECT
public:
  explicit AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  virtual ~AbstractPropertyWidget() = default;

  template<typename T> T configuration(const QString& key)
  {
    return Property::get_value<T>(m_properties, [key](const Property& p) {
      return p.configuration.get<T>(key);
    });
  }

public Q_SLOTS:
  void on_property_value_changed(Property* property);
  void update_enabledness();

protected:
  Scene& scene;
  void set_widget(std::unique_ptr<QWidget> widget);
  QString label() const;

  class LabelLayout : public QHBoxLayout
  {
  public:
    explicit LabelLayout();
    void set_label(const QString& label);
    template<typename T> void set_thing(std::unique_ptr<T> thing)
    {
      this->remove_old_thing();
      assert(count() == 1);
      assert(thing->parent() == nullptr);
      if constexpr (std::is_base_of_v<QWidget, T>) {
        addWidget(thing.release(), 1);
      } else if constexpr (std::is_base_of_v<QLayout, T>) {
        addLayout(thing.release(), 1);
      } else {
        Q_UNREACHABLE();
      }
    }

  private:
    QLabel* m_label = nullptr;

    using QHBoxLayout::addItem;
    using QHBoxLayout::addWidget;
    using QHBoxLayout::addLayout;
    using QHBoxLayout::removeItem;
    using QHBoxLayout::removeWidget;
    void remove_old_thing();
  };

protected Q_SLOTS:
  virtual void update_edit() = 0;

private:
  std::set<Property*> m_properties;
  template<typename PropertyT> friend class PropertyWidget;
  QTimer m_update_timer;
};

template<typename PropertyT>
class PropertyWidget : public AbstractPropertyWidget
{
public:
  using AbstractPropertyWidget::AbstractPropertyWidget;
  using property_type = PropertyT;
  using value_type = typename property_type::value_type;
  static const QString TYPE;

protected:
  virtual void set_properties_value(const value_type& value)
  {
    const auto is_noop = [&value](const Property* p) {
      return p->value<value_type>() == value;
    };
    if (!std::all_of(m_properties.begin(), m_properties.end(), is_noop)) {
      auto command = std::make_unique<PropertiesCommand<property_type>>(m_properties, value);
      scene.submit(std::move(command));
    }
  }

  auto get_properties_values() const
  {
    return ::transform<value_type>(m_properties, [](const auto* property) {
      return property->template value<value_type>();
    });
  }

  const std::set<Property*>& properties() const { return m_properties; }
  QString type() const override { return TYPE; }
};

template<typename PropertyT> const QString
PropertyWidget<PropertyT>::TYPE = QString(PropertyT::TYPE) + "Widget";

}  // namespace omm
