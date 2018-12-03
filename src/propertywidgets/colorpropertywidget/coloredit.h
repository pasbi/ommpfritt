#pragma once

#include <QWidget>

#include "color/color.h"
#include "propertywidgets/multivalueedit.h"

namespace omm
{

class ColorEdit : public QWidget, public MultiValueEdit<Color>
{
  Q_OBJECT
public:
  explicit ColorEdit();
  void set_value(const value_type& value) override;
  value_type value() const override;

Q_SIGNALS:
  void color_changed(const Color& color);

protected:
  void set_inconsistent_value() override;
  void paintEvent(QPaintEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  bool m_is_consistent = false;
  Color m_current_color = Color::BLACK;
};

}  // namespace omm
