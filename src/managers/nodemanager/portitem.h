#pragma once

#include <QGraphicsItem>
#include <QPicture>

namespace omm
{

class NodeItem;
class AbstractPort;

class PortItem : public QGraphicsItem
{
public:
  explicit PortItem(AbstractPort& port, NodeItem& parent);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* options, QWidget* widget) override;
  QRectF boundingRect() const override;
  static constexpr double height = 30.0;
  static constexpr double radius = 8.0;
  static constexpr auto TYPE = QGraphicsItem::UserType + 1;
  int type() const override { return TYPE; }
  NodeItem& node_item;
  AbstractPort& port;
};

}
