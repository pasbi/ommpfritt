#include "managers/curvemanager/curvemanager.h"
#include <QHeaderView>
#include <QEvent>
#include "scene/scene.h"
#include "scene/messagebox.h"
#include "managers/curvemanager/curvemanagertitlebar.h"
#include "managers/curvemanager/curvemanagerwidget.h"
#include "managers/curvemanager/curvemanagerquickaccessdelegate.h"
#include <QSplitter>
#include <QTreeView>
#include "animation/animator.h"
#include "managers/curvemanager/curvetree.h"

namespace omm
{

CurveManager::CurveManager(Scene& scene)
  : Manager(tr("Curves"), scene)
{
  setObjectName(TYPE);
  auto title_bar = std::make_unique<CurveManagerTitleBar>(*this);
  m_title_bar = title_bar.get();
  setTitleBarWidget(title_bar.release());

  auto curve_manager_widget = std::make_unique<CurveManagerWidget>(scene);
  m_widget = curve_manager_widget.get();

  auto tree_widget = std::make_unique<CurveTree>(scene);

  auto splitter = std::make_unique<QSplitter>(Qt::Horizontal);
  splitter->addWidget(tree_widget.release());
  splitter->addWidget(curve_manager_widget.release());

  set_widget(std::move(splitter));

}

bool CurveManager::perform_action(const QString& name)
{
  return false;
}

}  // namespace omm
