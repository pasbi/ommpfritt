#pragma once

#include <set>
#include <QWidget>
#include "managers/managertitlebar.h"

class QPushButton;

namespace omm
{

class CurveManager;
class AbstractPropertyOwner;

class CurveManagerTitleBar : public ManagerTitleBar
{
  Q_OBJECT
public:
  explicit CurveManagerTitleBar(CurveManager& parent);
  void set_selection(const std::set<AbstractPropertyOwner*>& selection);

private:
  QPushButton* m_lock_button;

private Q_SLOTS:
  void update_lock_button_icon(bool checked);
};

}  // namespace omm
