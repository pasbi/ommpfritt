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
  ~ColorDialog() override;
  ColorDialog(ColorDialog&&) = delete;
  ColorDialog(const ColorDialog&) = delete;
  ColorDialog& operator=(ColorDialog&&) = delete;
  ColorDialog& operator=(const ColorDialog&) = delete;

  static std::pair<Color, bool> get_color(const Color& old, QWidget* parent = nullptr);
  void set_color(const Color& color);
  [[nodiscard]] Color color() const;

private:
  ColorWidget* m_color_widget;
};

}  // namespace omm
