#pragma once

#include "aspects/propertyowner.h"
#include "nodesystem/nodecompiler.h"
#include "nodesystem/port.h"
#include "nodesystem/propertyport.h"
#include <QObject>
#include <QRectF>
#include <memory>

class QMenu;

namespace omm
{
class NodeModel;

class Node
    : public PropertyOwner<Kind::Node>
    , public AbstractFactory<QString, false, Node, NodeModel&>
{
  Q_OBJECT
public:
  explicit Node(NodeModel& model);
  ~Node() override;
  Node(Node&&) = delete;
  Node(const Node&) = default;
  Node& operator=(const Node&) = delete;
  Node& operator=(Node&&) = delete;

  Flag flags() const override
  {
    return Flag::None;
  }

  std::set<AbstractPort*> ports() const;
  template<typename PortT> std::set<PortT*> ports() const
  {
    const auto predicate = [](AbstractPort* p) { return p->port_type == PortT::PORT_TYPE; };
    const auto convert = [](AbstractPort* p) { return static_cast<PortT*>(p); };
    return ::transform<PortT*>(::filter_if(ports(), predicate), convert);
  }

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  void set_pos(const QPointF& pos);
  QPointF pos() const;

  bool is_free() const;
  NodeModel& model() const
  {
    return m_model;
  }
  QString name() const override;

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

  template<typename PortT> PortT* find_port(const Property& property) const
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

  template<typename PortT> PortT* find_port(const QString& key) const
  {
    return find_port<PortT>(*property(key));
  }

  virtual void populate_menu(QMenu&)
  {
  }
  virtual QString title() const;

  virtual QString output_data_type(const OutputPort& port) const;
  virtual QString input_data_type(const InputPort& port) const;
  virtual bool accepts_input_data_type(const QString& type, const InputPort& port) const;
  virtual bool is_valid() const;
  std::unique_ptr<Node> clone(NodeModel& model) const;

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

  template<typename PropertyT, typename... Args>
  PropertyT& create_property(const QString& key, PortType type, Args&&... args)
  {
    static_assert(std::is_base_of<Property, PropertyT>::value);
    auto property = std::make_unique<PropertyT>(std::forward<Args>(args)...);
    PropertyT& ref = *property;
    add_property(key, std::move(property), type);
    return ref;
  }

  template<typename PropertyT, typename... Args>
  PropertyT& create_property(const QString& key, Args&&... args)
  {
    return create_property<PropertyT>(key, PortType::Both, std::forward<Args>(args)...);
  }

  Property& add_property(const QString& key, std::unique_ptr<Property> property, PortType type);
  AbstractPort& add_port(std::unique_ptr<AbstractPort> port);
  std::unique_ptr<AbstractPort> remove_port(const AbstractPort& port);
  std::map<QString, AbstractPort*> named_ports;

public:
  Property& add_property(const QString& key, std::unique_ptr<Property> property) override;
  std::unique_ptr<Property> extract_property(const QString& key) override;
  virtual bool copyable() const
  {
    return true;
  }

Q_SIGNALS:
  void pos_changed(const QPointF& pos);
  void ports_changed();

private:
  QPointF m_pos;
  std::set<std::unique_ptr<AbstractPort>> m_ports;

  NodeModel& m_model;
  class ReferencePolisher;

public:
  struct Detail {
    std::map<AbstractNodeCompiler::Language, QString> definitions;
    std::vector<const char*> menu_path;
  };

  static const Detail& detail(const QString& name)
  {
    return *m_details.at(name);
  }
  static QString fst_con_ptype(const std::vector<InputPort*>& ports, const QString& default_t);

public:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
  static std::map<QString, const Detail*> m_details;
};

}  // namespace omm
