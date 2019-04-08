#pragma once

#include <QWidget>

#include "color/color.h"
#include "propertywidgets/multivalueedit.h"

namespace omm
{

class ColorEdit : public QWidget, public MultiValueEdit<Color>
{
public:
  explicit ColorEdit(const on_value_changed_t& on_value_changed);
  void set_value(const value_type& value) override;
  value_type value() const override;

protected:
  void set_inconsistent_value() override;
  void paintEvent(QPaintEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
  bool m_is_consistent = false;
  Color m_current_color = Colors::BLACK;
};

}  // namespace omm
