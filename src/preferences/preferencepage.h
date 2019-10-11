#pragma once

#include <QWidget>

namespace omm
{

class PreferencePage : public QWidget
{
public:
  using QWidget::QWidget;
  virtual ~PreferencePage() = default;
};

}  // namespace omm
