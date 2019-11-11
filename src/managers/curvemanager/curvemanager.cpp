#include "managers/curvemanager/curvemanager.h"
#include <QEvent>
#include "scene/scene.h"
#include "scene/messagebox.h"
#include "managers/curvemanager/curvemanagertitlebar.h"
#include "managers/curvemanager/curvemanagerwidget.h"

namespace omm
{

CurveManager::CurveManager(Scene& scene)
  : Manager(tr("Curves"), scene)
{
  setObjectName(TYPE);
  auto title_bar = std::make_unique<CurveManagerTitleBar>(*this);
  m_title_bar = title_bar.get();
  setTitleBarWidget(title_bar.release());

  connect(&scene.message_box(), SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          this, SLOT(set_selection(std::set<AbstractPropertyOwner*>)));

  auto curve_manager_widget = std::make_unique<CurveManagerWidget>(scene);
  m_widget = curve_manager_widget.get();
  set_widget(std::move(curve_manager_widget));
}

bool CurveManager::perform_action(const QString& name)
{
  return false;
}

void CurveManager::set_selection(const std::set<AbstractPropertyOwner*>& selection)
{
  if (!m_is_locked) {
    m_widget->set_selection(selection);
  }
}

}  // namespace omm
