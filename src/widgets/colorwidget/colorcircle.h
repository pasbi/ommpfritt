#pragma once

#include "widgets/colorwidget/colorpicker.h"
#include <QPainterPath>

namespace omm
{
class ColorCircle : public ColorPicker
{
  Q_OBJECT
public:
  explicit ColorCircle(QWidget* parent = nullptr);
  [[nodiscard]] QString name() const override
  {
    return tr("ColorCircle");
  }
  void set_color(const Color& color) override;
  void accept();

protected:
  void paintEvent(QPaintEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent*) override;

private:
  const QPainterPath m_circle;
  const QPainterPath m_triangle;

  void to_polar(const QPointF& pos, double& r, double& arg) const;
  void to_barycentric(const QPointF& pos, double& black, double& white) const;
  [[nodiscard]] QPointF to_relative(const QPoint& pos) const;

  bool m_drag_circle = false;
  bool m_drag_triangle = false;
  void set_color(const QPoint& p);
};

}  // namespace omm
