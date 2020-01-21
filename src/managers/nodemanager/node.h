#pragma once

#include <memory>
#include <QRectF>
#include <QObject>
#include "aspects/propertyowner.h"
#include "managers/nodemanager/port.h"
#include "managers/nodemanager/propertyport.h"
#include "managers/nodemanager/nodecompiler.h"
#include "keybindings/menu.h"

namespace omm
{

class NodeModel;
class Menu;

class Node
  : public PropertyOwner<Kind::Node>
  , public AbstractFactory<QString, Node, Scene*>
  , public ReferencePolisher
{
  Q_OBJECT
public:
  explicit Node(Scene* scene);
  ~Node();

  Flag flags() const override { return Flag::None; }

  std::set<AbstractPort*> ports() const;
  template<typename PortT> std::set<PortT*> ports() const
  {
    const auto predicate = [](AbstractPort* p) { return p->port_type == PortT::PORT_TYPE; };
    const auto convert = [](AbstractPort* p) { return static_cast<PortT*>(p); };
    return ::transform<PortT*>(::filter_if(ports(), predicate), convert);
  }

  void set_model(NodeModel* model);

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  void set_pos(const QPointF& pos);
  QPointF pos() const;

  bool is_free() const;
  NodeModel* model() const { return m_model; }
  QString name() const override;

  virtual QString definition() const { return ""; }
  QString uuid() const { return "node_definition_" + type(); }
  AbstractNodeCompiler::Language language() const;

  static constexpr auto POS_PTR = "pos";
  static constexpr auto CONNECTIONS_PTR = "connection";
  static constexpr auto OUTPUT_PORT_PTR = "out";
  static constexpr auto INPUT_PORT_PTR = "in";
  static constexpr auto CONNECTED_NODE_PTR = "node";

  std::set<Node*> successors() const;
  template<typename PortT> PortT* find_port(std::size_t index) const
  {
    for (auto&& port : m_ports) {
      if (port->index == index && PortT::PORT_TYPE == port->port_type) {
        return static_cast<PortT*>(port.get());
      }
    }
    return nullptr;
  }

  template<typename PortT> PropertyPort<PortT::PORT_TYPE>* find_port(const Property& property) const
  {
    for (auto&& port : m_ports) {
      if (PortT::PORT_TYPE == port->port_type && port->flavor == PortFlavor::Property) {
        auto* property_port = static_cast<PropertyPort<PortT::PORT_TYPE>*>(port.get());
        if (property_port->property() == &property) {
          return property_port;
        }
      }
    }
    return nullptr;
  }

  virtual void populate_menu(QMenu&) {}
  virtual QString title() const;

  virtual QString output_data_type(const OutputPort& port) const;
  virtual bool accepts_input_data_type(const QString& type, const InputPort& port) const;
  virtual bool is_valid() const;

protected:
  template<typename PortT, typename... Args> PortT& add_port(Args&&... args)
  {
    const std::size_t n = std::count_if(m_ports.begin(), m_ports.end(), [](auto&& port) {
      return port->port_type == PortT::PORT_TYPE;
    });
    auto port = std::make_unique<PortT>(*this, n, std::forward<Args>(args)...);
    auto& ref = *port;
    add_port(std::move(port));
    return ref;
  }

  AbstractPort& add_port(std::unique_ptr<AbstractPort> port);
  std::unique_ptr<AbstractPort> remove_port(const AbstractPort& port);

  Property& add_property(const QString& key, std::unique_ptr<Property> property) override;
  std::unique_ptr<Property> extract_property(const QString& key) override;

  std::map<QString, AbstractPort*> named_ports;

Q_SIGNALS:
  void pos_changed();

private:
  QPointF m_pos;
  NodeModel* m_model = nullptr;
  std::set<std::unique_ptr<AbstractPort>> m_ports;

  // Only required during deserialization.
  struct ConnectionIds {
    std::size_t input_port;
    std::size_t output_port;
    std::size_t node_id;
  };
  std::list<ConnectionIds> m_connection_ids;
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override;
  friend void register_nodes();

public:
  struct Detail
  {
    std::set<AbstractNodeCompiler::Language> languages;
  };

  static const Detail& detail(const QString& name) { return *m_details.at(name); }

private:
  static std::map<QString, const Detail*> m_details;
};

void register_nodes();

}  // namespace
