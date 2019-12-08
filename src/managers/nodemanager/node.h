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

  static constexpr auto POS_KEY = "pos";

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

};

void register_nodes();

}  // namespace
