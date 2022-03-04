#pragma once

#include "commands/propertycommand.h"
#include "common.h"
#include "properties/typedproperty.h"
#include "scene/history/macro.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QWidget>

namespace omm
{
class AbstractPropertyWidget
    : public QWidget
    , public AbstractFactory<QString,
                             false,
                             AbstractPropertyWidget,
                             Scene&,
                             const std::set<Property*>&>
{
  Q_OBJECT
public:
  explicit AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties);

  template<typename T> T configuration(const QString& key)
  {
    return Property::get_value<T>(m_properties,
                                  [key](const Property& p) { return p.configuration.get<T>(key); });
  }

  void on_property_value_changed(omm::Property* property) const;
  void update_enabledness();

protected:
  Scene& scene;
  void set_widget(std::unique_ptr<QWidget> widget);
  [[nodiscard]] QString label() const;

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
    using QHBoxLayout::addLayout;
    using QHBoxLayout::addWidget;
    using QHBoxLayout::removeItem;
    using QHBoxLayout::removeWidget;
    void remove_old_thing();
  };

protected:
  virtual void update_edit() = 0;
  virtual void update_configuration()
  {
  }
  void submit_command(std::unique_ptr<Command> command);

private:
  std::set<Property*> m_properties;
  template<typename PropertyT> friend class PropertyWidget;
  QTimer m_update_timer;
};

template<typename PropertyT> class PropertyWidget : public AbstractPropertyWidget
{
public:
  using AbstractPropertyWidget::AbstractPropertyWidget;
  using property_type = PropertyT;
  using value_type = typename property_type::value_type;
  static QString TYPE()
  {
    return QString(PropertyT::TYPE()) + "Widget";
  }

protected:
  virtual void set_properties_value(const value_type& value)
  {
    const auto is_noop = [&value](const Property* p) { return p->value<value_type>() == value; };
    if (!std::all_of(m_properties.begin(), m_properties.end(), is_noop)) {
      submit_command(std::make_unique<PropertiesCommand<property_type>>(m_properties, value));
    }
  }

  auto get_properties_values() const
  {
    return util::transform(m_properties, [](const auto* property) {
      return property->template value<value_type>();
    });
  }

  [[nodiscard]] const std::set<Property*>& properties() const
  {
    return m_properties;
  }
  [[nodiscard]] QString type() const override
  {
    return TYPE();
  }
};

}  // namespace omm
