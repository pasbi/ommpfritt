#pragma once

#include "color/color.h"
#include <QWidget>
#include <memory>

namespace Ui { class UiColorEdit; }

namespace omm
{

class UiColorEdit : public QWidget
{
public:
  explicit UiColorEdit(QWidget* parent = nullptr);
  ~UiColorEdit();
  void set_default_color(const Color& color);
  void set_color(const Color& color);
  Color color() const;

protected:
  void paintEvent(QPaintEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;

private:
  std::unique_ptr<Ui::UiColorEdit> m_ui;
  Color m_color;
  Color m_default_color;

};

}  // namespace omm
