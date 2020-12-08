#pragma once

#include <QWidget>

#include "color/color.h"
#include "propertywidgets/multivalueedit.h"

namespace omm
{
class ColorEdit
    : public QWidget
    , public MultiValueEdit<Color>
{
  Q_OBJECT
public:
  explicit ColorEdit();
  void set_value(const value_type& value) override;
  [[nodiscard]] value_type value() const override;
  QString text = "";

protected:
  void set_inconsistent_value() override;
  void paintEvent(QPaintEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;

Q_SIGNALS:
  void value_changed(omm::MultiValueEdit<omm::Color, std::equal_to<>>::value_type);

private:
  bool m_is_consistent = false;
  Color m_current_color = Colors::BLACK;
  QColor m_contrast_color;
  [[nodiscard]] QColor compute_contrast_color() const;
};

}  // namespace omm
