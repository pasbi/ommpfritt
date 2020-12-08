#pragma once

#include "preferences/preferencepage.h"
#include "preferences/preferences.h"
#include <map>

class QDoubleSpinBox;
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
  ~ViewportPage() override;
  ViewportPage(ViewportPage&&) = delete;
  ViewportPage(const ViewportPage&) = delete;
  ViewportPage& operator=(ViewportPage&&) = delete;
  ViewportPage& operator=(const ViewportPage&) = delete;

  void about_to_accept() override;

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
    QComboBox* m_cb_penstyle;
    QComboBox* m_cb_zorder;
    QDoubleSpinBox* m_sp_penwidth;
    Preferences::GridOption& m_model;
  };

  std::map<QString, MouseModifiersGroup> m_mouse_modifiers;
  std::map<QString, GridGroup> m_grid_options;
};

}  // namespace omm
