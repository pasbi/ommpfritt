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
#include "managers/nodemanager/node.h"
#include "managers/nodemanager/ordinaryport.h"
#include "managers/nodemanager/portitem.h"

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

}  // namespace

namespace omm
{

class PropertyWidgetItem : public QGraphicsProxyWidget
{
public:
  PropertyWidgetItem(QGraphicsItem* parent, std::unique_ptr<AbstractPropertyWidget> widget)
    : QGraphicsProxyWidget(parent), widget(widget.get())
  {
    setWidget(widget.release());
  }

protected:
  void focusInEvent(QFocusEvent* event) override
  {
    auto siblings = parentItem()->childItems();
    for (QGraphicsItem* sibling : siblings) {
      sibling->stackBefore(this);
    }

    QGraphicsProxyWidget::focusInEvent(event);
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
  painter->drawText(m_shape, Qt::AlignTop | Qt::AlignLeft, QString("%1, %2").arg(pos().x()).arg(pos().y()));

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
    node.set_pos(value.toPointF());
  }
  return value;
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
  std::set<OrdinaryPort<PortType::Input>*> ordinary_inputs;
  std::set<OrdinaryPort<PortType::Output>*> ordinary_outputs;
  for (AbstractPort* p : node.ports()) {
    if (p->flavor == PortFlavor::Property) {
      Property* property = p->port_type == PortType::Input
                        ? static_cast<PropertyInputPort&>(*p).property()
                        : static_cast<PropertyOutputPort&>(*p).property();
      const auto it = std::find_if(properties.begin(), properties.end(),
                                   [property](const PropertyPorts& pp)
      {
        return property == &pp.property;
      });
      if (it == properties.end()) {
        properties.push_back(PropertyPorts(*property));
      }
      if (p->port_type == PortType::Input) {
        it->i = static_cast<PropertyInputPort*>(p);
      } else {
        it->o = static_cast<PropertyOutputPort*>(p);
      }
    } else {
      if (p->port_type == PortType::Input) {
        ordinary_inputs.insert(static_cast<OrdinaryPort<PortType::Input>*>(p));
      } else {
        ordinary_outputs.insert(static_cast<OrdinaryPort<PortType::Output>*>(p));
      }
    }
  }

  static constexpr double header_height = 50;
  static constexpr double footer_height = 10;
  double pos_y = header_height;

  for (const PropertyPorts& pp : properties) {
    add_port(pp.i, pp.o, pos_y);
    add_property_widget(pp.property, pos_y);
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
  const auto remove_all = [this](auto&& items) {
    for (const auto& item : items) {
      model()->removeItem(item.get());
    }
    items.clear();
  };

  for (auto&& [type, items] : m_port_items) {
    remove_all(items);
  }
  remove_all(m_property_items);
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

void NodeItem::add_property_widget(Property& property, double pos_y)
{
  auto pw = AbstractPropertyWidget::make(property.widget_type(),
                                         *node.scene(),
                                         std::set { &property });
  pw->resize(pw->width(), PortItem::height);
  auto& ref = *pw;
  auto pw_item = std::make_unique<PropertyWidgetItem>(this, std::move(pw));

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
  pw_item->setY(pos_y - PortItem::height/2.0);
  m_property_items.insert(std::move(pw_item));
}

NodeModel* NodeItem::model() const
{
  return static_cast<NodeModel*>(QGraphicsItem::scene());
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
