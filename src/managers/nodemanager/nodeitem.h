#pragma once

#include <QGraphicsItem>
#include "managers/nodemanager/port.h"

namespace omm
{

class PropertyWidgetItem;
class Node;
class PropertyOutputPort;
class PropertyInputPort;
template<PortType> class OrdinaryPort;
class Property;
class PortItem;
class NodeScene;
class NodeView;

class NodeItem : public QGraphicsItem
{
public:
  explicit NodeItem(Node& node);
  ~NodeItem();
  QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
  PortItem* port_item(const AbstractPort& port) const;
  static constexpr auto TYPE = QGraphicsItem::UserType + 2;
  static constexpr double small_slot_height = 13.0;
  static constexpr double large_slot_height = 30.0;
  int type() const override { return TYPE; }
  Node& node;
  void toggle_expanded();

protected:
  QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
  void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

  template<typename PropertyT, typename... Args>
  PropertyT& create_property(const QString& key, Args&&... args)
  {
    static_assert(std::is_base_of<Property, PropertyT>::value);
    auto property = std::make_unique<PropertyT>(std::forward<Args>(args)...);
    PropertyT& ref = *property;
    add_property(key, std::move(property));
    return ref;
  }

private:
  bool m_is_expanded = false;
  void update_children();
  QRectF m_shape;

  void clear_ports();
  void align_ports();
  void add_port(PropertyInputPort* ip, PropertyOutputPort* op, double pos_y);
  void add_port(AbstractPort& p, double pos_y);
  void add_property_widget(Property& property, double pos_y, double height);
  void adjust_port_pos();
  NodeScene* scene() const;

  std::map<PortType, std::set<std::unique_ptr<PortItem>>> m_port_items;
  std::set<std::unique_ptr<PropertyWidgetItem>> m_property_items;

  struct Slot
  {
    const double pos_y;
    const PortType type;
    const QString text;
    void adjust(QRectF& rect) const;
    Qt::Alignment alignment() const;
  };

  std::list<Slot> m_slots;
  QObject m_context;
};

}  // namespace omm
