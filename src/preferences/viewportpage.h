#pragma once

#include "preferences/preferencepage.h"
#include "preferences/preferences.h"
#include <map>

class QComboBox;
class QFormLayout;
class QCheckBox;

namespace omm
{

class ViewportPage : public PreferencePage
{
  Q_OBJECT
public:
  explicit ViewportPage(Preferences& preferences);
  ~ViewportPage();

  void about_to_accept();

private:
  Preferences& m_preferences;

  class MouseModifiersGroup
  {
  public:
    MouseModifiersGroup(Preferences::MouseModifier& model, QFormLayout& layout);
    void apply();

  private:
    QComboBox* m_button_cb;
    std::map<Qt::KeyboardModifier, QCheckBox*> m_modifier_cbs;
    Preferences::MouseModifier& m_model;
  };

  class GridGroup
  {
  public:
    GridGroup(Preferences::GridOption& model, QFormLayout& layout);
    void apply();
  private:
    Preferences::GridOption& m_model;
  };

  std::map<QString, MouseModifiersGroup> m_mouse_modifiers;
  std::map<double, GridGroup> m_grid_options;
};

}  // namespace omm
