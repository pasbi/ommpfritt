#pragma once

#include <memory>
#include <QWidget>
#include "colorpicker.h"

namespace Ui { class ColorWidget; }

namespace omm
{

class AbstractColorComponentWidget;
class NamedColors;

class ColorWidget : public ColorPicker
{
  Q_OBJECT
public:
  explicit ColorWidget(QWidget* parent = nullptr);
  ~ColorWidget();
  std::string name() const override;
  void set_compact();
  void hide_named_colors();

public Q_SLOTS:
  void set_color(const Color& color) override;

private:
  std::unique_ptr<Ui::ColorWidget> m_ui;
  std::list<ColorPicker*> m_color_pickers;
  void add_color_picker(std::unique_ptr<ColorPicker> picker);
  std::map<Color::Role, std::list<AbstractColorComponentWidget*>> m_component_widgets;

  std::string m_color_name;

private Q_SLOTS:
  void show_named_colors_dialog();
};

}  // namespace omm
