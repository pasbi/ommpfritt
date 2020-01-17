#include "managers/nodemanager/nodes/referencenode.h"
#include "scene/messagebox.h"
#include "commands/forwardingportcommand.h"
#include "keybindings/menu.h"
#include "properties/referenceproperty.h"
#include "variant.h"
#include "scene/scene.h"
#include "properties/optionsproperty.h"
#include "managers/nodemanager/nodemodel.h"

namespace
{

QString to_string(omm::PortType port_type)
{
  switch (port_type) {
  case omm::PortType::Input:
  case omm::PortType::Output:
    return QT_TRANSLATE_NOOP("ReferenceNode", "output");
  default:
    Q_UNREACHABLE();
    return "";
  }
}

}  // namespace

namespace omm
{

const Node::Detail ReferenceNode::detail { { NodeCompiler::Language::Python,
                                             NodeCompiler::Language::GLSL } };

ReferenceNode::ReferenceNode(Scene* scene)
  : Node(scene)
{
  const QString category = tr("Node");
  create_property<ReferenceProperty>(REFERENCE_PROPERTY_KEY)
    .set_label(tr("Reference")).set_category(category);

  connect(&scene->message_box(), &MessageBox::property_value_changed,
          [this](AbstractPropertyOwner& owner, const QString& key, Property&)
  {
    if (&owner == property(REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>()) {
      if (key == AbstractPropertyOwner::NAME_PROPERTY_KEY) {
        model()->notify_node_shape_changed();
      }
    }
  });
}

void ReferenceNode::populate_menu(QMenu& menu)
{
  auto forward_menu = std::make_unique<Menu>(tr("Forwarded Ports"));
  AbstractPropertyOwner* apo = reference();
  if (apo == nullptr) {
    forward_menu->addAction(tr("No properties."))->setEnabled(false);
  } else {
    for (auto key : apo->properties().keys()) {
      Property* property = apo->property(key);
      if (language() == NodeCompiler::Language::Python) {
        auto property_menu = std::make_unique<Menu>(property->label());
        property_menu->addAction(make_property_action(PortType::Input, key,
                                                      tr("input", "ReferenceNode")).release());
        property_menu->addAction(make_property_action(PortType::Output, key,
                                                      tr("output", "ReferenceNode")).release());
        forward_menu->addMenu(property_menu.release());
      } else if (language() == NodeCompiler::Language::GLSL) {
        forward_menu->addAction(make_property_action(PortType::Output, key,
                                                     property->label()).release());
      }
    }
  }

  menu.addMenu(forward_menu.release());
}

QString ReferenceNode::title() const
{
  QString r_name = QObject::tr("< none >", "ReferenceLineEdit");
  if (AbstractPropertyOwner* apo = reference(); apo != nullptr) {
    r_name = apo->property(AbstractPropertyOwner::NAME_PROPERTY_KEY)->value<QString>();
  }
  return Node::title() + tr(" [%1]").arg(r_name);
}

void
ReferenceNode::deserialize(AbstractDeserializer& deserializer, const Serializable::Pointer& root)
{
  Node::deserialize(deserializer, root);
  for (auto type : { PortType::Input, PortType::Output }) {
    auto pointer = make_pointer(root, type == PortType::Input ? "input" : "output");
    std::set<QString> keys;
    deserializer.get(keys, pointer);
    for (const QString& key : keys) {
      add_forwarding_port(type, key);
    }
  }
}

void
ReferenceNode::serialize(AbstractSerializer& serializer, const Serializable::Pointer& root) const
{
  Node::serialize(serializer, root);
  for (auto [type, map] : m_forwarded_ports) {
    auto pointer = make_pointer(root, type == PortType::Input ? "input" : "output");
    serializer.set_value(::get_keys(map), pointer);
  }
}

void ReferenceNode::on_property_value_changed(Property* property)
{
  if (property == this->property(REFERENCE_PROPERTY_KEY)) {
    model()->notify_node_shape_changed();
  }
}

AbstractPropertyOwner* ReferenceNode::reference() const
{
  return property(REFERENCE_PROPERTY_KEY)->value<AbstractPropertyOwner*>();
}

std::unique_ptr<QAction>
ReferenceNode::make_property_action(PortType port_type, const QString& key, const QString& label)
{
  auto action = std::make_unique<QAction>(label);
  action->setCheckable(true);
  auto map = m_forwarded_ports[port_type];
  action->setChecked(map.find(key) != map.end());
  connect(action.get(), &QAction::triggered, [this, port_type, key](bool checked) {
    if (checked) {
      scene()->submit<AddForwardingPortCommand>(*this, port_type, key);
    } else {
      scene()->submit<RemoveForwardingPortCommand>(*this, port_type, key);
    }
  });
  return action;
}

AbstractPort& ReferenceNode::add_forwarding_port(PortType port_type, const QString& key)
{
  const auto get_property = [this, key]() -> Property* {
    AbstractPropertyOwner* reference = this->reference();
    if (reference == nullptr) {
      return nullptr;
    } else if (reference->has_property(key)) {
      return reference->property(key);
    } else {
      return nullptr;
    }
  };
  auto* port = [port_type, get_property, this]() -> AbstractPort* {
    switch (port_type) {
    case PortType::Input:
      return &add_port<PropertyPort<PortType::Input>>(get_property);
    case PortType::Output:
      return &add_port<PropertyPort<PortType::Output>>(get_property);
    default:
      Q_UNREACHABLE();
      return nullptr;
    }
  }();
  m_forwarded_ports[port_type][key] = port;
  return *port;
}

std::unique_ptr<AbstractPort>
ReferenceNode::remove_forwarding_port(PortType port_type, const QString& key)
{
  const auto it = m_forwarded_ports.at(port_type).find(key);
  auto owned = Node::remove_port(*it->second);
  m_forwarded_ports.at(port_type).erase(it);
  return owned;
}

}  // namespace
