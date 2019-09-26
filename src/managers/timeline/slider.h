#include <QWidget>

#pragma once

namespace omm
{

class Animator;

class Slider : public QWidget
{
  Q_OBJECT
public:
  explicit Slider(QWidget* parent = nullptr);

  std::pair<int, int> range() const { return std::pair(m_min, m_max); }
  void set_animator(Animator& animator);

public Q_SLOTS:
  void set_min(double frame);
  void set_max(double frame);

Q_SIGNALS:
  void value_changed(int);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  double m_min;
  double m_max;
  Animator* m_animator = nullptr;
  double frame_to_pixel(double frame) const;
  double pixel_to_frame(double pixel) const;

  static constexpr int left_margin = 5;
  static constexpr int right_margin = 5;
  static constexpr int top_margin  = 10;
  static constexpr int bottom_margin = 10;
  const QRectF content_rect() const;
  void draw_lines(QPainter& painter) const;
  void draw_current(QPainter& painter) const;
  double pixel_per_frame() const;

  bool m_pan_active;
  bool m_zoom_active;
  QPoint m_last_mouse_pos;
  QPoint m_mouse_down_pos;
};

}  // namespace
