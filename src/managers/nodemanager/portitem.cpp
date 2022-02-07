#include "managers/nodemanager/portitem.h"
#include "managers/nodemanager/nodeitem.h"
#include "nodesystem/port.h"
#include "preferences/uicolors.h"
#include <QPainter>

namespace
{
constexpr double pen_width = 2.0;

}  // namespace

namespace omm
{
PortItem::PortItem(nodes::AbstractPort& port, NodeItem& parent)
    : QGraphicsItem(&parent), node_item(parent), port(port)
{
}

void PortItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* widget)
{
  static constexpr QRectF ellipse(-radius, -radius, 2 * radius, 2 * radius);
  static constexpr double big_number = 10000.0;
  static constexpr double margin = 5.0;

  painter->setRenderHint(QPainter::Antialiasing);

  QPainterPath path;
  path.addEllipse(ellipse);
  const auto type_name = QString::fromStdString(std::string{variant_type_name(port.data_type())});
  QBrush brush(ui_color(*widget, "NodeView", QString("port-%1").arg(type_name)));
  painter->fillPath(path, brush);
  QPen pen;
  pen.setColor(Qt::black);
  pen.setWidth(pen_width);
  painter->setPen(pen);
  painter->drawPath(path);
  if (port.flavor == nodes::PortFlavor::Ordinary) {
    QRectF text_rect;
    const double h = painter->fontMetrics().height() * 2.0;
    text_rect.setHeight(h);
    text_rect.setY(h);
    int flags = Qt::AlignVCenter;
    if (port.port_type == nodes::PortType::Input) {
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
  static constexpr QRectF ellipse(-radius, -radius, 2 * radius, 2 * radius);
  return ellipse.adjusted(-pen_width, -pen_width, pen_width, pen_width);
}

}  // namespace omm
