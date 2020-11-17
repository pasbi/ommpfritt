#pragma once

#include "managers/managertitlebar.h"
#include <QWidget>
#include <set>

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
};

}  // namespace omm
