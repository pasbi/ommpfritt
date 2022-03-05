#include "managers/nodemanager/nodeitem.h"
#include "managers/nodemanager/nodescene.h"
#include "managers/nodemanager/nodeview.h"
#include "managers/nodemanager/portitem.h"
#include "nodesystem/node.h"
#include "nodesystem/nodemodel.h"
#include "nodesystem/ordinaryport.h"
#include "preferences/uicolors.h"
#include "propertywidgets/optionpropertywidget/optionpropertywidget.h"
#include "propertywidgets/optionpropertywidget/optionsedit.h"
#include "propertywidgets/propertywidget.h"
#include "removeif.h"
#include <QAbstractItemView>
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

namespace
{
const double node_pen_width = 2.2;

std::unique_ptr<QGraphicsItem> create_facade(const QGraphicsProxyWidget& widget_item)
{
  auto facade = std::make_unique<QGraphicsPixmapItem>(widget_item.parentItem());
  facade->setPos(widget_item.pos());
  QWidget* widget = widget_item.widget();
  QImage image(widget->size(), QImage::Format_ARGB32);
  widget->render(&image);
  facade->setPixmap(QPixmap::fromImage(image));
  return facade;
}

/**
 * From the docs:
 *    Note that an item will not receive double click events if it is neither selectable nor
 *    movable (single mouse clicks are ignored in this case, and that stops the generation of
 *    double clicks).
 */
template<class BaseItem> class DoubleClickableGraphicsItem : public BaseItem
{
public:
  using BaseItem::BaseItem;

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override
  {
    BaseItem::mousePressEvent(event);
    event->accept();
  }

  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
  {
    BaseItem::mouseReleaseEvent(event);
    event->accept();
  }
};

void stack_before_siblings(QGraphicsItem& item)
{
  const auto is_not_top_level_item = [](const auto* item) { return item->parentItem() != nullptr; };
  const auto siblings = is_not_top_level_item(&item)
                            ? item.parentItem()->childItems()
                            : util::remove_if(item.scene()->items(), is_not_top_level_item);
  for (QGraphicsItem* sibling : siblings) {
    sibling->stackBefore(&item);
  }
}

QWidget* leaf_widget(QWidget* widget, QPoint& pos)
{
  while (true) {
    QWidget* child = widget->childAt(pos);
    if (child == nullptr) {
      return widget;
    } else {
      widget = child;
      pos = child->mapFromParent(pos);
    }
  }
  Q_UNREACHABLE();
  return nullptr;
}

}  // namespace

namespace omm
{
class PropertyWidgetItem : public DoubleClickableGraphicsItem<QGraphicsProxyWidget>
{
public:
  PropertyWidgetItem(QGraphicsItem* parent, std::unique_ptr<AbstractPropertyWidget> widget)
      : DoubleClickableGraphicsItem(parent), widget(widget.get())
  {
    setWidget(widget.release());
  }

protected:
  void focusInEvent(QFocusEvent* event) override
  {
    stack_before_siblings(*this);
    QGraphicsProxyWidget::focusInEvent(event);
  }

  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override
  {
    if (widget != nullptr) {
      QPoint pos = event->pos().toPoint();
      QWidget* leaf_widget = ::leaf_widget(widget, pos);

      QMouseEvent mdbce(QEvent::MouseButtonDblClick,
                        pos,
                        event->button(),
                        event->buttons(),
                        event->modifiers());
      QApplication::sendEvent(leaf_widget, &mdbce);
    }
  }

public:
  AbstractPropertyWidget* const widget;
};

NodeItem::NodeItem(nodes::Node& node) : node(node)
{
  update_children();
  setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsScenePositionChanges);
  setPos(node.pos());
  QObject::connect(&node, &nodes::Node::pos_changed, &m_context, [this](const QPointF& pos) {
    setPos(pos);
  });
  QObject::connect(&node, &nodes::Node::ports_changed, &m_context, [this]() { update_children(); });
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  QTimer::singleShot(0, &node, [this]() { update_children(); });
}

NodeItem::~NodeItem() = default;

QRectF NodeItem::boundingRect() const
{
  const double hnpw = node_pen_width / 2.0;
  return m_shape.adjusted(-hnpw, -hnpw, hnpw, hnpw);
}

void NodeItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* widget)
{
  static constexpr int CORNER_RADIUS = 10;
  painter->setRenderHint(QPainter::Antialiasing);
  QPainterPath path;
  path.addRoundedRect(m_shape, CORNER_RADIUS, CORNER_RADIUS, Qt::AbsoluteSize);

  painter->fillPath(path, Qt::gray);

  painter->setPen(Qt::black);
  painter->drawText(m_shape, Qt::AlignTop | Qt::AlignHCenter, node.title());

  const auto draw_outline = [&path, painter](const QColor& color, double width) {
    QPen pen;
    pen.setColor(color);
    pen.setWidthF(width);
    painter->save();
    painter->setPen(pen);
    painter->drawPath(path);
    painter->restore();
  };

  draw_outline(isSelected() ? Qt::yellow : Qt::black, node_pen_width);
  if (!node.is_valid()) {
    draw_outline(ui_color(*widget, "NodeView", "node-outline-valid"), node_pen_width / 2.0);
  }

  const double hmargin = PortItem::radius + 5;
  for (auto&& [pos_y, slot] : m_slots) {
    QRectF rect = boundingRect();

    rect.setTop(pos_y - small_slot_height / 2.0);
    rect.setHeight(small_slot_height);
    rect.adjust(hmargin, 0, -hmargin, 0);
    slot.draw(*painter, rect);
  }
}

PortItem* NodeItem::port_item(const nodes::AbstractPort& port) const
{
  for (nodes::PortType type : {nodes::PortType::Input, nodes::PortType::Output}) {
    if (const auto it = m_port_items.find(type); it != m_port_items.end()) {
      for (const auto& port_item : it->second) {
        if (&port == &port_item->port) {
          return port_item.get();
        }
      }
    }
  }
  return nullptr;
}

void NodeItem::toggle_expanded()
{
  const bool will_expand = !m_is_expanded && can_expand();
  if (will_expand != m_is_expanded) {
    m_is_expanded = will_expand;
    update_children();
  }
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == ItemPositionChange) {
    node.set_pos(value.toPointF());
  }
  return value;
}

void NodeItem::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
  // don't accept the drag on the node itself. Drags may be accepted on children.
  event->ignore();
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
  if (scene()->itemAt(event->scenePos(), QTransform()) == this) {
    stack_before_siblings(*this);
    toggle_expanded();
  } else {
    QGraphicsItem::mouseDoubleClickEvent(event);
  }
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  stack_before_siblings(*this);
  QGraphicsItem::mousePressEvent(event);
}

void NodeItem::update_children()
{
  prepareGeometryChange();
  clear_ports();
  struct PropertyPorts {
    explicit PropertyPorts(Property* property) : property(property), i(nullptr), o(nullptr)
    {
    }
    Property* property;
    nodes::PropertyPort<nodes::PortType::Input>* i;
    nodes::PropertyPort<nodes::PortType::Output>* o;
  };

  std::list<PropertyPorts> property_ports;
  std::list<nodes::OrdinaryPort<nodes::PortType::Input>*> ordinary_inputs;
  std::list<nodes::OrdinaryPort<nodes::PortType::Output>*> ordinary_outputs;
  auto ports = util::transform<std::vector>(node.ports());
  std::sort(ports.begin(), ports.end(), [](const auto* const a, const auto* const b) {
    return a->index < b->index;
  });
  for (auto* const p : ports) {
    if (p->flavor == nodes::PortFlavor::Property) {
      Property* property = p->port_type == nodes::PortType::Input
                               ? dynamic_cast<nodes::PropertyPort<nodes::PortType::Input>&>(*p).property()
                               : dynamic_cast<nodes::PropertyPort<nodes::PortType::Output>&>(*p).property();
      const auto match_property = [property](const PropertyPorts& pp) { return property == pp.property; };
      const auto it = std::find_if(property_ports.begin(), property_ports.end(), match_property);
      PropertyPorts* current = nullptr;
      if (it == property_ports.end()) {
        property_ports.emplace_back(property);
        current = &property_ports.back();
      } else {
        current = &*it;
      }

      if (p->port_type == nodes::PortType::Input) {
        current->i = dynamic_cast<nodes::PropertyPort<nodes::PortType::Input>*>(p);
      } else {
        current->o = dynamic_cast<nodes::PropertyPort<nodes::PortType::Output>*>(p);
      }
    } else {
      if (p->port_type == nodes::PortType::Input) {
        ordinary_inputs.push_back(dynamic_cast<nodes::OrdinaryPort<nodes::PortType::Input>*>(p));
      } else {
        ordinary_outputs.push_back(dynamic_cast<nodes::OrdinaryPort<nodes::PortType::Output>*>(p));
      }
    }
  }

  static constexpr double header_height = 30;
  static constexpr double footer_height = 0;

  double pos_y = header_height;
  const double slot_height = m_is_expanded ? large_slot_height : small_slot_height;

  for (const PropertyPorts& pp : property_ports) {
    add_port(pp.i, pp.o, pos_y);
    if (pp.property != nullptr) {
      if (m_is_expanded) {
        add_property_widget(*pp.property, pos_y, slot_height);
      } else {
        m_slots[pos_y].get_center_text = [pp]() { return pp.property->label(); };
      }
    } else {
      m_slots[pos_y].get_center_text = []() { return "Undefined."; };
    }
    pos_y += slot_height;
  }

  double input_pos_y = pos_y;
  double output_pos_y = pos_y;

  for (auto* op : ordinary_outputs) {
    add_port(*op, output_pos_y);
    m_slots[output_pos_y].get_right_text = [op]() { return op->label(); };
    output_pos_y += small_slot_height;
  }

  for (auto* ip : ordinary_inputs) {
    add_port(*ip, input_pos_y);
    m_slots[input_pos_y].get_left_text = [ip]() { return ip->label(); };
    input_pos_y += small_slot_height;
  }

  const double height = std::max(output_pos_y, input_pos_y) + footer_height;
  const double width = m_is_expanded ? 300 : 150;
  m_shape = QRectF(-width / 2, 0, width, height);

  align_ports();
}

void NodeItem::clear_ports()
{
  const auto remove_all = [this](auto&& items) {
    for (const auto& item : items) {
      scene()->removeItem(item.get());
    }
    items.clear();
  };

  for (auto&& [type, items] : m_port_items) {
    remove_all(items);
  }
  remove_all(m_property_items);
  m_slots.clear();
}

void NodeItem::align_ports()
{
  for (const auto& port_item : m_port_items[nodes::PortType::Input]) {
    port_item->setX(m_shape.left());
  }

  for (const auto& port_item : m_port_items[nodes::PortType::Output]) {
    port_item->setX(m_shape.right());
  }

  const double margin = PortItem::radius + 5;
  for (const auto& item : m_property_items) {
    QWidget* w = item->widget;
    w->resize(static_cast<int>(m_shape.width() - 2.0 * margin), w->height());
    item->setX(-w->width() / 2.0);
  }
}

void NodeItem::add_port(nodes::AbstractPort& p, double pos_y)
{
  auto port_item = std::make_unique<PortItem>(p, *this);
  port_item->setY(pos_y);
  m_port_items[p.port_type].insert(std::move(port_item));
}

void NodeItem::add_property_widget(Property& property, double pos_y, double height)
{
  auto pw
      = AbstractPropertyWidget::make(property.widget_type(), *node.scene(), std::set{&property});
  pw->resize(pw->width(), static_cast<int>(height));
  auto& ref = *pw;
  auto pw_item = std::make_unique<PropertyWidgetItem>(this, std::move(pw));
  setAcceptDrops(true);
  pw_item->setAcceptDrops(true);

  if (ref.type() == OptionPropertyWidget::TYPE()) {
    auto* combobox = dynamic_cast<OptionPropertyWidget*>(&ref)->combobox();
    combobox->prevent_popup = true;
    QObject::connect(combobox, &OptionsEdit::popup_shown, combobox, [pw_item = pw_item.get(), combobox]() {
      NodeView* view = []() {
        QWidget* w = QApplication::widgetAt(QCursor::pos());
        assert(w != nullptr);
        return qobject_cast<NodeView*>(w->parentWidget());
      }();
      const QPointF scene_pos = pw_item->mapToScene(QPointF());
      const QPoint global_pos = view->mapToGlobal(view->mapFromScene(scene_pos));
      auto facade = create_facade(*pw_item);
      QWidget* widget = pw_item->widget;
      pw_item->setWidget(nullptr);
      combobox->QComboBox::showPopup();
      combobox->view()->parentWidget()->move(global_pos);
      auto connection_destroyer = std::make_unique<QObject>();
      auto& cd_ref = *connection_destroyer;
      QObject::connect(combobox,
                       &OptionsEdit::popup_hidden,
                       &cd_ref,
                       [pw_item,
                        widget,
                        facade = std::move(facade),
                        cd = std::move(connection_destroyer)]() mutable {
                         pw_item->setWidget(widget);
                         facade->scene()->removeItem(facade.get());
                         facade.reset();
                         cd.reset();
                       });
    });
  }
  pw_item->setY(pos_y - height / 2.0);
  m_property_items.insert(std::move(pw_item));
}

NodeScene* NodeItem::scene() const
{
  return dynamic_cast<NodeScene*>(QGraphicsItem::scene());
}

bool NodeItem::can_expand() const
{
  const auto ports = node.ports();
  return std::any_of(ports.begin(), ports.end(), [](const auto* const port) {
    return port->flavor == nodes::PortFlavor::Property;
  });
}

void NodeItem::add_port(nodes::PropertyPort<nodes::PortType::Input>* const ip,
                        nodes::PropertyPort<nodes::PortType::Output>* const op,
                        const double pos_y)
{
  if (ip != nullptr) {
    add_port(*ip, pos_y);
  }
  if (op != nullptr) {
    add_port(*op, pos_y);
  }
}

void NodeItem::Slot::draw(QPainter& painter, const QRectF& slot_rect) const
{
  assert((get_left_text || get_right_text) != !!get_center_text);
  if (get_left_text) {
    QRectF rect = slot_rect;
    if (get_right_text) {
      rect.setRight(0);
    }
    painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, get_left_text());
  }
  if (get_right_text) {
    QRectF rect = slot_rect;
    if (get_left_text) {
      rect.setLeft(0);
    }
    painter.drawText(rect, Qt::AlignVCenter | Qt::AlignRight, get_right_text());
  }
  if (get_center_text) {
    const auto draw_maybe = [&painter, y = slot_rect.center().y()](double x1, double x2) {
      if (x1 < x2) {
        painter.drawLine(static_cast<int>(x1),
                         static_cast<int>(y),
                         static_cast<int>(x2),
                         static_cast<int>(y));
      }
    };
    QRectF actual;
    const QString center_text = get_center_text();
    painter.drawText(slot_rect, Qt::AlignVCenter | Qt::AlignCenter, center_text, &actual);
    painter.save();
    static constexpr int PEN_COLOR_ALPHA = 50;
    painter.setPen(QColor(0, 0, 0, PEN_COLOR_ALPHA));
    if (!center_text.isEmpty()) {
      static constexpr int MARGIN = 5;
      draw_maybe(slot_rect.left(), actual.left() - MARGIN);
      draw_maybe(actual.right() + MARGIN, slot_rect.right());
    }
    painter.restore();
  }
}

}  // namespace omm
