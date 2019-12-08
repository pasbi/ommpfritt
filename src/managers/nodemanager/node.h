#pragma once

#include <memory>
#include <QRectF>
#include <QObject>

#include "managers/nodemanager/port.h"

namespace omm
{

class NodeModel;

class Node : public QObject
{
  Q_OBJECT
public:
  explicit Node();
  ~Node();

  std::set<Port*> ports() const;

  void set_model(NodeModel* model);
  template<typename PortT, typename... Args> void add_port(Args&&... args)
  {
    const std::size_t n = std::count_if(m_ports.begin(), m_ports.end(), [](auto&& port) {
      return port->is_input == PortT::IS_INPUT;
    });
    auto port = std::make_unique<PortT>(*this, n, std::forward<Args...>(args...));
    m_ports.insert(std::move(port));
  }

  void set_pos(const QPointF& pos);
  QPointF pos() const;

Q_SIGNALS:
  void pos_changed();

private:
  QPointF m_pos;
  NodeModel* m_model;
  std::set<std::unique_ptr<Port>> m_ports;

};

}  // namespace
