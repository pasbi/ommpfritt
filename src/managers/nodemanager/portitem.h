#pragma once

#include <QGraphicsItem>
#include <QPicture>

namespace omm
{

class NodeItem;

namespace nodes
{
class AbstractPort;
}  // namespace nodes

class PortItem : public QGraphicsItem
{
public:
  explicit PortItem(nodes::AbstractPort& port, NodeItem& parent);

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* options, QWidget* widget) override;
  [[nodiscard]] QRectF boundingRect() const override;
  static constexpr auto TYPE = QGraphicsItem::UserType + 1;
  static constexpr double radius = 6;
  [[nodiscard]] int type() const override
  {
    return TYPE;
  }
  NodeItem& node_item;
  nodes::AbstractPort& port;
};

}  // namespace omm
