#pragma once

#include "preferences/preferencepage.h"
#include <memory>

namespace Ui { class GeneralPage; }

namespace omm
{

class GeneralPage : public PreferencePage
{
  Q_OBJECT
public:
  explicit GeneralPage();
  ~GeneralPage();
  void about_to_reject() override;
  void about_to_accept() override;

private:
  std::unique_ptr<Ui::GeneralPage> m_ui;
  const std::vector<std::string> m_available_languages;
};

}  // namespace omm
