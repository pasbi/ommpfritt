#include "managers/curvemanager/curvemanagertitlebar.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <memory>
#include "managers/curvemanager/curvemanager.h"
#include "menuhelper.h"

namespace omm
{

CurveManagerTitleBar::CurveManagerTitleBar(CurveManager& parent)
  : ManagerTitleBar(parent)
{
  add_widget(make_lock_button());
  add_widget(std::make_unique<QLabel>(parent.windowTitle()));
}

}  // namespace omm
