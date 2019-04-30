#include "managers/historymanager/historymanager.h"
#include "scene/history/historymodel.h"
#include <QListView>
#include "scene/scene.h"

namespace omm
{

HistoryManager::HistoryManager(Scene &scene)
  : Manager(tr("History"), scene)
  , m_model(scene.history)
{
  setObjectName(TYPE);
  auto view = std::make_unique<QListView>();
  m_view = view.get();
  set_widget(std::move(view));
  m_view->setModel(&m_model);
  connect(m_view, &QListView::doubleClicked, [&scene](const QModelIndex& index) {
    scene.history.set_index(index.row());
  });
  m_view->setSelectionMode(QAbstractItemView::NoSelection);
}

std::string HistoryManager::type() const { return TYPE; }

}  // namespace omm
