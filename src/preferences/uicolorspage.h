#pragma once

#include "color/color.h"
#include "preferences/uicolors.h"
#include <memory>
#include "preferences/preferencepage.h"

namespace Ui { class UiColorsPage; }

namespace omm
{

class UiColors;

class UiColorsPage : public PreferencePage
{
public:
  explicit UiColorsPage(UiColors& colors);
  ~UiColorsPage();

  void about_to_accept() override;
  void about_to_reject() override;

private:
  std::unique_ptr<Ui::UiColorsPage> m_ui;
};

}  // namespace omm
