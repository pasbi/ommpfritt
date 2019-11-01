#pragma once

#include "preferences/preferencepage.h"
#include "preferences/preferences.h"
#include <memory>

namespace Ui { class GeneralPage; }

namespace omm
{

class GeneralPage : public PreferencePage
{
  Q_OBJECT
public:
  explicit GeneralPage(Preferences& preferences);
  ~GeneralPage();
  void about_to_accept() override;

private:
  const std::vector<QString> m_available_languages;
  Preferences& m_preferences;

  std::unique_ptr<Ui::GeneralPage> m_ui;
};

}  // namespace omm
