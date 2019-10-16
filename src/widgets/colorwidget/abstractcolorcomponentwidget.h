#pragma once

#include <QWidget>
#include "color/color.h"

namespace omm
{

class AbstractColorComponentWidget : public QWidget
{
  Q_OBJECT
public:
  using QWidget::QWidget;
  virtual void set_role(Color::Role role);
  Color color() const { return m_color; }
  Color::Role role() const { return m_role; }

Q_SIGNALS:
  void color_changed(const Color& color);

public Q_SLOTS:
  virtual void set_color(const Color& color);

private:
  Color m_color;
  Color::Role m_role;
  void set_value(int x);

};

}  // namespace omm
