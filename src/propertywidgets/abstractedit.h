#pragma once

#include <QWidget>

namespace omm
{
class AbstractEdit : public QWidget
{
  Q_OBJECT
public:
  using QWidget::QWidget;

Q_SIGNALS:
  void value_changed();
};

}  // namespace omm
