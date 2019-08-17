#pragma once

#include <QWidget>
#include "properties/typedproperty.h"
#include "common.h"
#include "scene/scene.h"
#include "commands/propertycommand.h"
#include <QTimer>

namespace omm
{

class AbstractPropertyWidget
  : public QWidget
  , public AbstractFactory< std::string, AbstractPropertyWidget,
                            Scene&, const std::set<Property*>& >
{
  Q_OBJECT
public:
  explicit AbstractPropertyWidget(Scene& scene, const std::set<Property*>& properties);
  virtual ~AbstractPropertyWidget() = default;

public Q_SLOTS:
  void on_property_value_changed(Property* property);

protected:
  void set_default_layout(std::unique_ptr<QWidget> other);
  Scene& scene;

protected Q_SLOTS:
  virtual void update_edit() = 0;

private:
  const std::string m_label;
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
  static const std::string TYPE;

protected:
  virtual void set_properties_value(const value_type& value)
  {
    const bool wrap = std::any_of(m_properties.begin(), m_properties.end(), [](const Property* p) {
      return p->wrap_with_macro;
    });

    const auto is_noop = [&value](const Property* p) {
      return p->value<value_type>() == value;
    };
    if (!std::all_of(m_properties.begin(), m_properties.end(), is_noop)) {
      auto command = std::make_unique<PropertiesCommand<property_type>>(m_properties, value);
      std::unique_ptr<HistoryModel::Macro> macro;
      if (wrap) {
        macro = scene.history.start_macro(QString::fromStdString(command->label()));
        for (auto* property : m_properties) { property->pre_submit(*property); }
      }
      scene.submit(std::move(command));
      if (wrap) {
        for (auto* property : m_properties) { property->post_submit(*property); }
      }
    }
  }

  auto get_properties_values() const
  {
    return ::transform<value_type>(m_properties, [](const auto* property) {
      return property->template value<value_type>();
    });
  }

  const std::set<Property*>& properties() const { return m_properties; }
};

template<typename PropertyT> const std::string
PropertyWidget<PropertyT>::TYPE = std::string(PropertyT::TYPE) + "Widget";

void register_propertywidgets();

}  // namespace omm
