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
  [[nodiscard]] QRectF boundingRect() const override;
  static constexpr auto TYPE = QGraphicsItem::UserType + 1;
  static constexpr double radius = 6;
  [[nodiscard]] int type() const override
  {
    return TYPE;
  }
  NodeItem& node_item;
  AbstractPort& port;
};

}  // namespace omm
