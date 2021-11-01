#pragma once

#include "preferences/preferencepage.h"
#include <memory>

namespace Ui
{
class GeneralPage;
}

namespace omm
{
class Preferences;
class GeneralPage : public PreferencePage
{
  Q_OBJECT
public:
  explicit GeneralPage(Preferences& preferences);
  ~GeneralPage() override;
  GeneralPage(GeneralPage&&) = delete;
  GeneralPage(const GeneralPage&) = delete;
  GeneralPage& operator=(GeneralPage&&) = delete;
  GeneralPage& operator=(const GeneralPage&) = delete;
  void about_to_accept() override;

private:
  const std::vector<QString> m_available_languages;
  Preferences& m_preferences;

  std::unique_ptr<Ui::GeneralPage> m_ui;
};

}  // namespace omm
