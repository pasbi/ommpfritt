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
  void set_default_values(int index);

private:
  std::unique_ptr<Ui::UiColorsPage> m_ui;
  UiColors& m_colors;

  void update_combobox();
  std::vector<std::pair<QString, std::string>> m_skins;
};

}  // namespace omm
