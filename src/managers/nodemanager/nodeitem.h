#pragma once

#include <QGraphicsItem>
#include "managers/nodemanager/port.h"

class QGraphicsProxyWidget;

namespace omm
{

class Node;
class PropertyOutputPort;
class PropertyInputPort;
template<PortType> class OrdinaryPort;
class Property;
class PortItem;
class NodeScene;

class NodeItem : public QGraphicsItem
{
public:
  explicit NodeItem(Node& node);
  ~NodeItem();
  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  PortItem* port_item(const AbstractPort& port) const;

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
  Node& m_node;
  void update_children();
  QRectF m_shape;

  void clear_ports();
  void align_ports();
  void add_port(PropertyInputPort* ip, PropertyOutputPort* op, double pos_y);
  void add_port(AbstractPort& p, double pos_y);
  void add_property_widget(Property& property, double pos_y);
  void adjust_port_pos();
  NodeScene* scene() const;

  std::map<PortType, std::set<std::unique_ptr<PortItem>>> m_port_items;
  std::set<std::unique_ptr<QGraphicsProxyWidget>> m_centered_items;
  QObject m_context;
};

}  // namespace omm
