#pragma once

#include "color/color.h"
#include <QWidget>

namespace omm
{
class AbstractColorComponentWidget : public QWidget
{
  Q_OBJECT
public:
  using QWidget::QWidget;
  virtual void set_role(Color::Role role);
  [[nodiscard]] Color color() const
  {
    return m_color;
  }

  [[nodiscard]] Color::Role role() const
  {
    return m_role;
  }

Q_SIGNALS:
  void color_changed(const omm::Color& color);

public:
  virtual void set_color(const omm::Color& color);

private:
  Color m_color;
  Color::Role m_role;
  void set_value(int x);
};

}  // namespace omm
