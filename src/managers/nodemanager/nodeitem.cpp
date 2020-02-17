#include "managers/nodemanager/nodeitem.h"
#include "managers/nodemanager/nodescene.h"
#include "preferences/uicolors.h"
#include "propertywidgets/propertywidget.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/ordinaryport.h"
#include "managers/nodemanager/portitem.h"

namespace
{

const double node_pen_width = 2.2;

}  // namespace

namespace omm
{

NodeItem::NodeItem(Node& node)
  : m_node(node)
{
  update_children();
  setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsScenePositionChanges);
  setPos(node.pos());
  QObject::connect(&node, &Node::pos_changed, &m_context, [this](const QPointF& pos) {
    setPos(pos);
  });
}

NodeItem::~NodeItem()
{
}

QRectF NodeItem::boundingRect() const
{
  const double hnpw = node_pen_width/2.0;
  return m_shape.adjusted(-hnpw, -hnpw, hnpw, hnpw);
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
  QPainterPath path;
  path.addRoundedRect(m_shape, 10, 10, Qt::AbsoluteSize);

  painter->fillPath(path, Qt::gray);

  const auto draw_outline = [&path, painter](const QColor& color, double width) {
    QPen pen;
    pen.setColor(color);
    pen.setWidthF(width);
    painter->setPen(pen);
    painter->drawPath(path);
  };

  draw_outline(isSelected() ? Qt::yellow : Qt::black, node_pen_width);
  if (!m_node.is_valid()) {
    draw_outline(ui_color(*widget, "NodeView", "node-outline-valid"), node_pen_width/2.0);
  }
}

PortItem* NodeItem::port_item(const AbstractPort& port) const
{
  for (PortType type : { PortType::Input, PortType::Output }) {
    if (const auto it = m_port_items.find(type); it != m_port_items.end()) {
      for (auto& port_item : it->second) {
        if (&port == &port_item->port) {
          return port_item.get();
        }
      }
    }
  }
  return nullptr;
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == ItemPositionChange) {
    m_node.set_pos(value.toPointF());
  }
  return value;
}

void NodeItem::update_children()
{
  prepareGeometryChange();
  clear_ports();
  std::map<Property*, std::pair<PropertyInputPort*, PropertyOutputPort*>> properties;
  std::set<OrdinaryPort<PortType::Input>*> ordinary_inputs;
  std::set<OrdinaryPort<PortType::Output>*> ordinary_outputs;
  for (AbstractPort* p : m_node.ports()) {
    if (p->flavor == PortFlavor::Property) {
      Property* property = p->port_type == PortType::Input
                        ? static_cast<PropertyInputPort&>(*p).property()
                        : static_cast<PropertyOutputPort&>(*p).property();

      if (const auto it = properties.find(property); it == properties.end()) {
        properties.insert({ property, { nullptr, nullptr } });
      }
      if (p->port_type == PortType::Input) {
        properties[property].first = static_cast<PropertyInputPort*>(p);
      } else {
        properties[property].second = static_cast<PropertyOutputPort*>(p);
      }
    }
  }

  static constexpr double header_height = 50;
  static constexpr double footer_height = 10;
  double pos_y = header_height;

  for (auto [property, ports]: properties) {
    add_port(ports.first, ports.second, pos_y);
    add_property_widget(*property, pos_y);
    pos_y += PortItem::height;
  }

  double input_pos_y = pos_y;
  double output_pos_y = pos_y;

  for (auto* op : ordinary_outputs) {
    add_port(*op, output_pos_y);
    output_pos_y += PortItem::height;
  }

  for (auto* ip : ordinary_inputs) {
    add_port(*ip, input_pos_y);
    input_pos_y += PortItem::height;
  }

  const double height = std::max(output_pos_y, input_pos_y) + footer_height;
  const double width = 300;
  m_shape = QRectF(-width/2, 0, width, height);

  align_ports();
}

void NodeItem::clear_ports()
{
  const auto remove_all = [scene=this->scene()](auto&& items) {
    for (const auto& item : items) {
      scene->removeItem(item.get());
    }
    items.clear();
  };

  for (auto&& [type, items] : m_port_items) {
    remove_all(items);
  }
  remove_all(m_centered_items);
}

void NodeItem::align_ports()
{
  for (auto& port_item : m_port_items[PortType::Input]) {
    port_item->setX(m_shape.left());
  }

  for (auto& port_item : m_port_items[PortType::Output]) {
    port_item->setX(m_shape.right());
  }

  const double margin = PortItem::radius + 5;
  for (auto& item : m_centered_items) {
    QWidget* w = item->widget();
    w->resize(m_shape.width() - 2 * margin, w->height());
    item->setX(-w->width()/2.0);
  }
}

void NodeItem::add_port(AbstractPort& p, double pos_y)
{
  auto port_item = std::make_unique<PortItem>(p, *this);
  port_item->setY(pos_y);
  m_port_items[p.port_type].insert(std::move(port_item));
}

void NodeItem::add_property_widget(Property& property, double pos_y)
{
  auto pw_item = std::make_unique<QGraphicsProxyWidget>(this);
  auto pw = AbstractPropertyWidget::make(property.widget_type(),
                                         *m_node.scene(),
                                         std::set { &property });

  pw->resize(pw->width(), PortItem::height);
  pw_item->setWidget(pw.release());
  pw_item->setY(pos_y - PortItem::height/2.0);
  m_centered_items.insert(std::move(pw_item));
}

NodeScene* NodeItem::scene() const
{
  return static_cast<NodeScene*>(QGraphicsItem::scene());
}

void NodeItem::add_port(PropertyInputPort* ip, PropertyOutputPort* op, double pos_y)
{
  if (ip != nullptr) {
    add_port(*ip, pos_y);
  }
  if (op != nullptr) {
    add_port(*op, pos_y);
  }
}

}  // namespace omm
