#pragma once

#include "color/color.h"
#include <QWidget>

namespace omm
{
class Color;

class ColorPicker : public QWidget
{
  Q_OBJECT
public:
  using QWidget::QWidget;
  [[nodiscard]] Color color() const;
  [[nodiscard]] virtual QString name() const = 0;

public:
  virtual void set_color(const omm::Color& color);

Q_SIGNALS:
  void color_changed(const omm::Color& color);

private:
  Color m_color;
};

}  // namespace omm
