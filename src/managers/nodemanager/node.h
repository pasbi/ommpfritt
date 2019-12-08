#pragma once

#include <memory>
#include <QRectF>
#include <QObject>
#include "aspects/propertyowner.h"
#include "managers/nodemanager/port.h"

namespace omm
{

class NodeModel;

class Node
  : public PropertyOwner<AbstractPropertyOwner::Kind::Node>
  , public AbstractFactory<QString, Node, Scene*>
  , public ReferencePolisher
{
  Q_OBJECT
public:
  explicit Node(Scene* scene);
  ~Node();

  AbstractPropertyOwner::Flag flags() const override { return AbstractPropertyOwner::Flag::None; }

  std::set<Port*> ports() const;

  void set_model(NodeModel* model);

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  void set_pos(const QPointF& pos);
  QPointF pos() const;

  bool is_free() const;

  static constexpr auto POS_PTR = "pos";
  static constexpr auto CONNECTIONS_PTR = "connection";
  static constexpr auto OUTPUT_PORT_PTR = "out";
  static constexpr auto INPUT_PORT_PTR = "in";
  static constexpr auto CONNECTED_NODE_PTR = "node";

  std::set<Node*> successors() const;
  template<typename PortT> PortT* find_port(std::size_t index) const
  {
    for (auto&& port : m_ports) {
      if (port->index == index && PortT::IS_INPUT == port->is_input) {
        assert(PortT::IS_INPUT == port->is_input);
        return static_cast<PortT*>(port.get());
      }
    }
    return nullptr;
  }

protected:
  template<typename PortT, typename... Args> void add_port(Args&&... args)
  {
    const std::size_t n = std::count_if(m_ports.begin(), m_ports.end(), [](auto&& port) {
      return port->is_input == PortT::IS_INPUT;
    });
    auto port = std::make_unique<PortT>(*this, n, std::forward<Args...>(args...));
    m_ports.insert(std::move(port));
  }

Q_SIGNALS:
  void pos_changed();

private:
  QPointF m_pos;
  NodeModel* m_model;
  std::set<std::unique_ptr<Port>> m_ports;

  // Only required during deserialization.
  struct ConnectionIds {
    std::size_t input_port;
    std::size_t output_port;
    std::size_t node_id;
  };
  std::list<ConnectionIds> m_connection_ids;
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override;
};

void register_nodes();

}  // namespace
