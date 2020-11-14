#pragma once

#include <QFontComboBox>

namespace omm
{
class FontComboBox : public QFontComboBox
{
public:
  explicit FontComboBox(QWidget* parent = nullptr);

protected:
  void wheelEvent(QWheelEvent* e) override;
};

}  // namespace omm
