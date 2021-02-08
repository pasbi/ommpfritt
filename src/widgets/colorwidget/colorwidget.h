#pragma once

#include "colorpicker.h"
#include <QWidget>
#include <memory>

namespace Ui
{
class ColorWidget;
}

namespace omm
{
class AbstractColorComponentWidget;
class NamedColors;

class ColorWidget : public ColorPicker
{
  Q_OBJECT
public:
  explicit ColorWidget(QWidget* parent = nullptr);
  ~ColorWidget() override;
  ColorWidget(ColorWidget&&) = delete;
  ColorWidget& operator=(ColorWidget&&) = delete;
  ColorWidget(const ColorWidget&) = delete;
  ColorWidget& operator=(const ColorWidget&) = delete;
  [[nodiscard]] QString name() const override;
  void set_compact();
  void hide_named_colors();

public:
  void set_color(const omm::Color& color) override;

private:
  std::unique_ptr<Ui::ColorWidget> m_ui;
  std::list<ColorPicker*> m_color_pickers;
  void add_color_picker(std::unique_ptr<ColorPicker> picker);
  std::map<Color::Role, std::list<AbstractColorComponentWidget*>> m_component_widgets;

private:
  void show_named_colors_dialog();
};

}  // namespace omm
