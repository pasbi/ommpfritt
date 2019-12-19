#include "managers/nodemanager/nodes/referencenode.h"
#include "keybindings/menu.h"
#include "properties/referenceproperty.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"

namespace omm
{

ReferenceNode::ReferenceNode(Scene* scene)
  : Node(scene)
{
  const QString category = tr("Node");
  create_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_label(tr("Reference")).set_category(category);
}

std::unique_ptr<Menu> ReferenceNode::make_menu()
{
  auto menu = std::make_unique<Menu>(tr("Forwarded Ports"));
  AbstractPropertyOwner* apo = reference();
  if (apo == nullptr) {
    menu->addAction(tr("No properties."))->setEnabled(false);
  } else {
    for (auto key : apo->properties().keys()) {
      Property* property = apo->property(key);
      auto property_menu = std::make_unique<Menu>(property->label());
      property_menu->addAction(make_property_action<PortType::Input>(tr("Input"), key).release());
      property_menu->addAction(make_property_action<PortType::Output>(tr("Output"), key).release());
      menu->addMenu(property_menu.release());
    }
  }
  return menu;
}

void ReferenceNode::on_property_value_changed(Property* property)
{
}

AbstractPropertyOwner* ReferenceNode::reference() const
{
  return property(REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>();
}

template<PortType port_type> std::unique_ptr<QAction>
ReferenceNode::make_property_action(const QString& label, const QString& key)
{
  const auto get_property_getter = [this](const QString& key) {
    return [this, key]() -> Property* {
      AbstractPropertyOwner* reference = this->reference();
      if (reference == nullptr) {
        return nullptr;
      } else if (reference->has_property(key)) {
        return reference->property(key);
      } else {
        return nullptr;
      }
    };
  };
  auto action = std::make_unique<QAction>(label);
  action->setCheckable(true);
  auto map = m_forwarded_ports[port_type];
  action->setChecked(map.find(key) != map.end());
  connect(action.get(), &QAction::triggered, [this, get_property_getter, key](bool checked) {
    if (checked) {
      AbstractPort& port = add_port<PropertyPort<port_type>>(get_property_getter(key));
      m_forwarded_ports[port_type][key] = &port;
    } else {
      remove_port(*m_forwarded_ports[port_type][key]);
    }
  });
  return action;
}

}  // namespace

template std::unique_ptr<QAction>
omm::ReferenceNode::make_property_action<omm::PortType::Input>(const QString&, const QString&);

template std::unique_ptr<QAction>
omm::ReferenceNode::make_property_action<omm::PortType::Output>(const QString&, const QString&);
