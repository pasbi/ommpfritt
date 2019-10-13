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
  Q_OBJECT
public:
  explicit UiColorsPage(UiColors& colors);
  ~UiColorsPage();

  void about_to_accept() override;
  void about_to_reject() override;

public Q_SLOTS:
  void load_skin(int index);

private:
  std::unique_ptr<Ui::UiColorsPage> m_ui;
  UiColors& m_colors;

  void update_combobox();
};

}  // namespace omm
