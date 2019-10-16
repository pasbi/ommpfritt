#pragma once

#include <QWidget>
#include "color/color.h"

namespace omm
{

class Color;

class ColorPicker : public QWidget
{
  Q_OBJECT
public:
  using QWidget::QWidget;
  Color color() const;
  virtual std::string name() const = 0;

public Q_SLOTS:
  virtual void set_color(const Color& color);

Q_SIGNALS:
  void color_changed(const Color& color) const;

private:
  Color m_color;

};

}  // namespace
