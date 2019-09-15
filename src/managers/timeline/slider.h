#include <QWidget>

#pragma once

namespace omm
{

class Slider : public QWidget
{
  Q_OBJECT
public:
  explicit Slider(QWidget* parent = nullptr);

  std::pair<int, int> range() const { return std::pair(m_min, m_max); }
  int value() const { return m_value; }

public Q_SLOTS:
  void set_value(int frame);
  void set_min(int frame);
  void set_max(int frame);

Q_SIGNALS:
  void value_changed(int);
  void min_changed(int);
  void max_changed(int);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  int m_min;
  int m_max;
  int m_value;
  bool m_handle_grabbed = false;
  double frame_to_pixel(int frame) const;
  int pixel_to_frame(int pixel) const;

  static constexpr int left_margin = 10;
  static constexpr int right_margin = 10;
  static constexpr int top_margin  = 10;
  static constexpr int bottom_margin = 10;
  const QRectF content_rect() const;
  void draw_lines(QPainter& painter) const;
  void draw_current(QPainter& painter) const;
  double pixel_per_frame() const;
};

}  // namespace
