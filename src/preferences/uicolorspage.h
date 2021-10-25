#pragma once

#include "preferences/preferencepage.h"
#include <memory>

namespace Ui
{
class UiColorsPage;
}

namespace omm
{
class UiColors;

class UiColorsPage : public PreferencePage
{
  Q_OBJECT
public:
  explicit UiColorsPage(UiColors& colors);
  UiColorsPage(UiColorsPage&&) = delete;
  UiColorsPage(const UiColorsPage&) = delete;
  UiColorsPage& operator=(UiColorsPage&&) = delete;
  UiColorsPage& operator=(const UiColorsPage&) = delete;
  ~UiColorsPage() override;

  void about_to_accept() override;
  void about_to_reject() override;

public:
  void load_skin(int index);

private:
  std::unique_ptr<Ui::UiColorsPage> m_ui;
  UiColors& m_colors;

  void update_combobox();
};

}  // namespace omm
