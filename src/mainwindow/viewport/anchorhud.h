#pragma once

#include "main/options.h"
#include "mainwindow/viewport/headupdisplay.h"
#include <QObject>
#include <QRectF>

class QWidget;

namespace omm
{
class AnchorHUD
    : public QObject
    , public HeadUpDisplay
{
  using Anchor = Options::Anchor;
  Q_OBJECT
public:
  explicit AnchorHUD(QWidget& widget);
  [[nodiscard]] QSize size() const override;
  void draw(QPainter& painter) const override;
  bool mouse_press(QMouseEvent& event) override;
  void mouse_release(QMouseEvent& event) override;
  bool mouse_move(QMouseEvent& event) override;
  [[nodiscard]] Anchor anchor() const
  {
    return m_anchor;
  }
  constexpr static double ANCHOR_RADIUS = 5;

Q_SIGNALS:
  void anchor_changed(Options::Anchor anchor);

public:
  void set_anchor(const Options::Anchor& anchor);

private:
  static constexpr std::array<Anchor, 5> PROPER_ANCHORS{Anchor::TopLeft,
                                                        Anchor::BottomLeft,
                                                        Anchor::TopRight,
                                                        Anchor::BottomRight,
                                                        Anchor::Center};

  [[nodiscard]] QRectF anchor_grid() const;
  QWidget& m_widget;
  Anchor m_anchor;
  void draw_anchor(QPainter& painter, const QRectF& grid, Anchor anchor) const;
  QPoint m_mouse_pos;
  Anchor m_disable_hover_for = Anchor::None;
  bool m_disable_hover = true;
};

}  // namespace omm
