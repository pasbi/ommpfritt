#include "managers/nodemanager/nodeitem.h"
#include <QAbstractItemView>
#include "managers/nodemanager/nodeview.h"
#include "propertywidgets/optionspropertywidget/optionsedit.h"
#include "propertywidgets/optionspropertywidget/optionspropertywidget.h"
#include "managers/nodemanager/nodemodel.h"
#include "preferences/uicolors.h"
#include "propertywidgets/propertywidget.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QApplication>
#include <QGraphicsSceneDragDropEvent>
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/ordinaryport.h"
#include "managers/nodemanager/portitem.h"
#include "managers/nodemanager/nodescene.h"

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
    Q_UNUSED(event);
    BaseItem::mousePressEvent(event);
    event->accept();
  }

  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override
  {
    Q_UNUSED(event)
    BaseItem::mouseReleaseEvent(event);
    event->accept();
  }

};

void stack_before_siblings(QGraphicsItem& item)
{
  const auto is_top_level_item = [](const auto* item) { return item->parentItem() == nullptr; };
  const auto siblings = is_top_level_item(&item) ? ::filter_if(item.scene()->items(), is_top_level_item)
                                                 : item.parentItem()->childItems();
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

      QMouseEvent mdbce(QEvent::MouseButtonDblClick, pos, event->button(),
                        event->buttons(), event->modifiers());
      qApp->sendEvent(leaf_widget, &mdbce);
    }
  }

public:
  AbstractPropertyWidget* const widget;
};

NodeItem::NodeItem(Node& node)
  : node(node)
{
  update_children();
  setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsScenePositionChanges);
  setPos(node.pos());
  QObject::connect(&node, &Node::pos_changed, &m_context, [this](const QPointF& pos) {
    setPos(pos);
  });
  QObject::connect(&node, &Node::ports_changed, &m_context, [this]() {
    update_children();
  });
  QTimer::singleShot(0, [this]() { update_children(); });
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
  painter->setRenderHint(QPainter::Antialiasing);
  QPainterPath path;
  path.addRoundedRect(m_shape, 10, 10, Qt::AbsoluteSize);

  painter->fillPath(path, Qt::gray);
  painter->setPen(Qt::black);
  painter->drawText(m_shape, Qt::AlignTop | Qt::AlignHCenter, node.title());

  const auto draw_outline = [&path, painter](const QColor& color, double width) {
    QPen pen;
    pen.setColor(color);
    pen.setWidthF(width);
    painter->setPen(pen);
    painter->drawPath(path);
  };

  draw_outline(isSelected() ? Qt::yellow : Qt::black, node_pen_width);
  if (!node.is_valid()) {
    draw_outline(ui_color(*widget, "NodeView", "node-outline-valid"), node_pen_width/2.0);
  }

  const double hmargin = PortItem::radius + 5;
  for (const Slot& slot : m_slots) {
    QRectF rect = boundingRect();
    rect.setTop(slot.pos_y - small_slot_height/2.0);
    rect.setHeight(small_slot_height);
    rect.adjust(hmargin, 0, -hmargin, 0);
    slot.adjust(rect);
    painter->setPen(Qt::black);
    QRectF actual_text_rect;
    painter->drawText(rect, Qt::AlignVCenter | slot.alignment(), slot.text, &actual_text_rect);
    if (slot.type == PortType::Both && !slot.text.isEmpty()) {
      const auto draw_maybe = [painter, y=rect.center().y()](double x1, double x2) {
        if (x1 < x2) {
          painter->drawLine(x1, y, x2, y);
        }
      };
      painter->save();
      painter->setPen(QColor(0, 0, 0, 50));
      draw_maybe(rect.left(), actual_text_rect.left() - 5);
      draw_maybe(actual_text_rect.right() + 5, rect.right());
      painter->restore();
    }
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

void NodeItem::toggle_expanded()
{
  m_is_expanded = !m_is_expanded;
  update_children();
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
    explicit PropertyPorts(Property& property)
      : property(property), i(nullptr), o(nullptr) {}
    Property& property;
    PropertyInputPort* i;
    PropertyOutputPort* o;
  };

  std::list<PropertyPorts> properties;
  std::list<OrdinaryPort<PortType::Input>*> ordinary_inputs;
  std::list<OrdinaryPort<PortType::Output>*> ordinary_outputs;
  auto ports = ::transform<AbstractPort*, std::vector>(node.ports());
  std::sort(ports.begin(), ports.end(), [](const AbstractPort* a, const AbstractPort* b) {
    return a->index < b->index;
  });
  for (AbstractPort* p : ports) {
    if (p->flavor == PortFlavor::Property) {
      Property* property = p->port_type == PortType::Input
                        ? static_cast<PropertyInputPort&>(*p).property()
                        : static_cast<PropertyOutputPort&>(*p).property();
      const auto it = std::find_if(properties.begin(), properties.end(),
                                   [property](const PropertyPorts& pp)
      {
        return property == &pp.property;
      });
      PropertyPorts* current = nullptr;
      if (it == properties.end() && property != nullptr) {
        properties.push_back(PropertyPorts(*property));
        current = &properties.back();
      } else {
        current = &*it;
      }

      if (p->port_type == PortType::Input) {
        current->i = static_cast<PropertyInputPort*>(p);
      } else {
        current->o = static_cast<PropertyOutputPort*>(p);
      }
    } else {
      if (p->port_type == PortType::Input) {
        ordinary_inputs.push_back(static_cast<OrdinaryPort<PortType::Input>*>(p));
      } else {
        ordinary_outputs.push_back(static_cast<OrdinaryPort<PortType::Output>*>(p));
      }
    }
  }

  static constexpr double header_height = 30;
  static constexpr double footer_height = 0;

  double pos_y = header_height;
  const double slot_height = m_is_expanded ? large_slot_height : small_slot_height;

  for (const PropertyPorts& pp : properties) {
    add_port(pp.i, pp.o, pos_y);
    if (m_is_expanded) {
      add_property_widget(pp.property, pos_y, slot_height);
    } else {
      m_slots.push_back({ pos_y, PortType::Both, pp.property.label() });
    }
    pos_y += slot_height;
  }

  double input_pos_y = pos_y;
  double output_pos_y = pos_y;

  for (auto* op : ordinary_outputs) {
    add_port(*op, output_pos_y);
    m_slots.push_back({ output_pos_y, PortType::Output, op->label() });
    output_pos_y += small_slot_height;
  }

  for (auto* ip : ordinary_inputs) {
    add_port(*ip, input_pos_y);
    m_slots.push_back({ input_pos_y, PortType::Input, ip->label() });
    input_pos_y += small_slot_height;
  }

  const double height = std::max(output_pos_y, input_pos_y) + footer_height;
  const double width = m_is_expanded ? 300 : 150;
  m_shape = QRectF(-width/2, 0, width, height);

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
  for (auto& port_item : m_port_items[PortType::Input]) {
    port_item->setX(m_shape.left());
  }

  for (auto& port_item : m_port_items[PortType::Output]) {
    port_item->setX(m_shape.right());
  }

  const double margin = PortItem::radius + 5;
  for (auto& item : m_property_items) {
    QWidget* w = item->widget;
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

void NodeItem::add_property_widget(Property& property, double pos_y, double height)
{
  auto pw = AbstractPropertyWidget::make(property.widget_type(),
                                         *node.scene(),
                                         std::set { &property });
  pw->resize(pw->width(), height);
  auto& ref = *pw;
  auto pw_item = std::make_unique<PropertyWidgetItem>(this, std::move(pw));
  setAcceptDrops(true);
  pw_item->setAcceptDrops(true);

  if (ref.type() == OptionsPropertyWidget::TYPE) {
    auto combobox = static_cast<OptionsPropertyWidget*>(&ref)->combobox();
    combobox->prevent_popup = true;
    QObject::connect(combobox, &OptionsEdit::popup_shown, [pw_item=pw_item.get(), combobox]() {
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
      auto connection_destroyer = new QObject();
      QObject::connect(combobox, &OptionsEdit::popup_hidden, connection_destroyer,
                       [pw_item, widget,
                        facade=std::move(facade),
                        connection_destroyer]() mutable
      {
        pw_item->setWidget(widget);
        facade->scene()->removeItem(facade.get());
        facade.reset();
        delete connection_destroyer;
      });
    });
  }
  pw_item->setY(pos_y - height/2.0);
  m_property_items.insert(std::move(pw_item));
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

void NodeItem::Slot::adjust(QRectF& rect) const
{
  if (type == PortType::Input) {
    rect.setRight(0);
  } else if (type == PortType::Output) {
    rect.setLeft(0);
  }
}

Qt::Alignment NodeItem::Slot::alignment() const
{
  switch (type) {
  case PortType::Input:
    return Qt::AlignLeft;
  case PortType::Output:
    return Qt::AlignRight;
  case PortType::Both:
    return Qt::AlignCenter;
  default:
    Q_UNREACHABLE();
    return 0;
  }
}

}  // namespace omm
