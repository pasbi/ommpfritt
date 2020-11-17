#include "managers/curvemanager/curvemanagertitlebar.h"
#include "managers/curvemanager/curvemanager.h"
#include "menuhelper.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <memory>

namespace omm
{
CurveManagerTitleBar::CurveManagerTitleBar(CurveManager& parent) : ManagerTitleBar(parent)
{
  add_widget(make_lock_button());
  add_widget(std::make_unique<QLabel>(parent.windowTitle()));
}

}  // namespace omm
