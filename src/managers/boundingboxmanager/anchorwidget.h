#pragma once

#include <QWidget>
#include "geometry/vec2.h"

namespace omm
{

class AnchorWidget : public QWidget
{
  Q_OBJECT
public:
  enum class Anchor { TopLeft, BottomLeft, TopRight, BottomRight, Center, None };
  AnchorWidget(QWidget* parent = nullptr);
  Anchor anchor() const;
  void set_anchor(const Anchor& anchor);
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void enterEvent(QEvent *event) override;
  void leaveEvent(QEvent *event) override;
  QSize minimumSizeHint() const override;
  Vec2f anchor_position(const QRectF& grid) const;

Q_SIGNALS:
  void anchor_changed(Anchor anchor);

protected:
  void paintEvent(QPaintEvent* e) override;

private:
  Anchor m_value = Anchor::Center;
  void draw_anchor(QPainter &painter, const QRectF& grid, Anchor anchor) const;
  QRectF anchor_grid() const;
  QPointF m_mouse_pos;
  QRectF anchor_rect(const QPointF& pos) const;
  static QPointF anchor_position(const QRectF& grid, const Anchor& anchor);
  static constexpr std::array<Anchor, 5> PROPER_ANCHORS { Anchor::TopLeft, Anchor::BottomLeft,
                                                          Anchor::TopRight, Anchor::BottomRight,
                                                          Anchor::Center };
  Anchor m_disable_hover_for = Anchor::None;
  bool m_disable_hover = true;
  const double ANCHOR_RADIUS = 5;

};

}  // namespace omm
