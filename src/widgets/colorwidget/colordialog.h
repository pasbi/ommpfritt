#pragma once

#include "color/color.h"
#include <QDialog>
#include <memory>

namespace omm
{
class ColorWidget;

class ColorDialog : public QDialog
{
public:
  explicit ColorDialog(QWidget* parent = nullptr);
  ~ColorDialog();

  static std::pair<Color, bool> get_color(const Color& old, QWidget* parent = nullptr);
  void set_color(const Color& color);
  Color color() const;

private:
  ColorWidget* m_color_widget;
};

}  // namespace omm
