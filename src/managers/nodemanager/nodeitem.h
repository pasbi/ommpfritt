#pragma once

#include "nodesystem/port.h"
#include <QGraphicsItem>

namespace omm
{

class PortItem;
class PropertyWidgetItem;
class Property;
class NodeScene;
class NodeView;

namespace nodes
{
class Node;
template<PortType> class OrdinaryPort;
template<PortType> class PropertyPort;
}  // namespace nodes

class NodeItem : public QGraphicsItem
{
public:
  explicit NodeItem(nodes::Node& node);
  ~NodeItem() override;
  NodeItem(NodeItem&&) = delete;
  NodeItem(const NodeItem&) = delete;
  NodeItem& operator=(NodeItem&&) = delete;
  NodeItem& operator=(const NodeItem&) = delete;
  [[nodiscard]] QRectF boundingRect() const override;
  void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* widget) override;
  [[nodiscard]] PortItem* port_item(const nodes::AbstractPort& port) const;
  static constexpr auto TYPE = QGraphicsItem::UserType + 2;
  static constexpr double small_slot_height = 13.0;
  static constexpr double large_slot_height = 30.0;
  [[nodiscard]] int type() const override
  {
    return TYPE;
  }
  nodes::Node& node;
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
  void add_port(nodes::PropertyPort<nodes::PortType::Input>* ip,
                nodes::PropertyPort<nodes::PortType::Output>* op,
                double pos_y);
  void add_port(nodes::AbstractPort& p, double pos_y);
  void add_property_widget(Property& property, double pos_y, double height);
  void adjust_port_pos();
  [[nodiscard]] NodeScene* scene() const;
  [[nodiscard]] bool can_expand() const;

  std::map<nodes::PortType, std::set<std::unique_ptr<PortItem>>> m_port_items;
  std::set<std::unique_ptr<PropertyWidgetItem>> m_property_items;

  struct Slot {
    using get_text_f = std::function<QString()>;
    Slot() = default;
    get_text_f get_left_text;
    get_text_f get_right_text;
    get_text_f get_center_text;
    void draw(QPainter& painter, const QRectF& slot_rect) const;
  };

  std::map<double, Slot> m_slots;
  QObject m_context;
};

}  // namespace omm
