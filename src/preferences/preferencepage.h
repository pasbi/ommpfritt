#pragma once

#include <QWidget>

namespace omm
{

class PreferencePage : public QWidget
{
public:
  using QWidget::QWidget;
  virtual ~PreferencePage() = default;
  virtual void about_to_accept() = 0;
  virtual void about_to_reject() = 0;
};

}  // namespace omm
