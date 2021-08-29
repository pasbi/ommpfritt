#include "managers/curvemanager/curvemanager.h"
#include "animation/animator.h"
#include "managers/curvemanager/curvemanagertitlebar.h"
#include "managers/curvemanager/curvemanagerwidget.h"
#include "managers/curvemanager/curvetreeview.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include <QEvent>
#include <QHeaderView>
#include <QSplitter>
#include <QTreeView>

namespace omm
{

CurveManager::CurveManager(Scene& scene) : Manager(tr("Curves"), scene)
{
  auto title_bar = std::make_unique<CurveManagerTitleBar>(*this);
  m_title_bar = title_bar.get();
  setTitleBarWidget(title_bar.release());

  auto tree_widget = std::make_unique<CurveTreeView>(scene);
  auto curve_manager_widget = std::make_unique<CurveManagerWidget>(scene, *tree_widget);
  m_widget = curve_manager_widget.get();

  auto splitter = std::make_unique<QSplitter>(Qt::Horizontal);
  splitter->addWidget(tree_widget.release());
  splitter->addWidget(curve_manager_widget.release());

  set_widget(std::move(splitter));
}

bool CurveManager::perform_action(const QString& name)
{
  Q_UNUSED(name)
  return false;
}

}  // namespace omm
