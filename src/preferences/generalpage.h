#pragma once

#include "preferences/preferencepage.h"
#include "preferences/preferences.h"
#include <memory>
#include <map>

namespace Ui { class GeneralPage; }

class QComboBox;
class QGridLayout;
class QCheckBox;

namespace omm
{

class GeneralPage : public PreferencePage
{
  Q_OBJECT
public:
  explicit GeneralPage(Preferences& preferences);
  ~GeneralPage();
  void about_to_reject() override;
  void about_to_accept() override;

private:
  const std::vector<QString> m_available_languages;
  QComboBox* m_cb_language;
  Preferences& m_preferences;


  class MouseModifiersGroup
  {
  public:
    MouseModifiersGroup(Preferences::MouseModifier& model, QGridLayout& layout, int& row);
    void apply();
  private:
    QComboBox* m_button_cb;
    std::map<Qt::KeyboardModifier, QCheckBox*> m_modifier_cbs;
    Preferences::MouseModifier& m_model;
  };

  std::map<QString, MouseModifiersGroup> m_mouse_modifiers;
};

}  // namespace omm
