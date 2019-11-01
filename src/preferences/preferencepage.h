#pragma once

#include <QWidget>

namespace omm
{

class PreferencePage : public QWidget
{
public:
  using QWidget::QWidget;
  virtual ~PreferencePage() = default;
  virtual void about_to_accept() {};
  virtual void about_to_reject() {};
};

}  // namespace omm
