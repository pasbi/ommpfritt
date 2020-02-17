#include "managers/nodemanager/portitem.h"
#include "managers/nodemanager/nodeitem.h"
#include "preferences/uicolors.h"
#include "managers/nodemanager/port.h"
#include <QPainter>

namespace
{

constexpr double pen_width = 2.0;

}  // namespace

namespace omm
{

PortItem::PortItem(AbstractPort& port, NodeItem& parent)
  : QGraphicsItem(&parent)
  , node_item(parent)
  , port(port)
{
}

void PortItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* options, QWidget* widget)
{
  static constexpr QRectF ellipse( -radius,  -radius,
                                  2*radius, 2*radius);
  static constexpr double big_number = 10000.0;
  static constexpr double margin = 10.0;
  QPainterPath path;
  path.addEllipse(ellipse);
  QBrush brush(ui_color(*widget, "NodeView", QString("port-%1").arg(port.data_type())));
  painter->fillPath(path, brush);
  QPen pen;
  pen.setColor(Qt::black);
  pen.setWidth(pen_width);
  painter->setPen(pen);
  painter->drawPath(path);
  if (port.flavor == PortFlavor::Ordinary) {
    QRectF text_rect;
    text_rect.setHeight(height*2);
    text_rect.setY(-height*2);
    int flags = Qt::AlignVCenter;
    if (port.port_type == PortType::Input) {
      text_rect.setLeft(ellipse.right() + margin);
      text_rect.setRight(big_number);
      flags |= Qt::AlignLeft;
    } else {
      text_rect.setRight(ellipse.left() - margin);
      text_rect.setLeft(-big_number);
      flags |= Qt::AlignRight;
    }
    QRectF text_bounding_rect;
    painter->drawText(text_rect, flags, port.label(), &text_bounding_rect);
  }
}

QRectF PortItem::boundingRect() const
{
  static constexpr QRectF ellipse( -radius,  -radius,
                                  2*radius, 2*radius);
  return ellipse.adjusted(-pen_width, -pen_width, pen_width, pen_width);
}

}  // namespace omm
