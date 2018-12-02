#include "managers/stylemanager/stylelistadapter.h"

#include <QMenu>
#include <QContextMenuEvent>
#include "menuhelper.h"
#include "managers/stylemanager/stylelistview.h"
#include "commands/newstylecommand.h"

namespace omm
{

StyleListView::StyleListView(Scene& scene)
  : m_scene(scene)
{
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setDragEnabled(true);
  setDefaultDropAction(Qt::MoveAction);
  viewport()->setAcceptDrops(true);
}

StyleListView::~StyleListView()
{
  set_model(nullptr); // unregister observer
}

void StyleListView::set_model(StyleListAdapter* model)
{
  // TODO same code as ObjectTreeView::set_model
  if (this->model()) {
    this->model()->scene().Observed<AbstractSelectionObserver>::unregister_observer(*this);
  }
  QListView::setModel(model);
  if (this->model()) {
    auto& scene = this->model()->scene();
    scene.Observed<AbstractSelectionObserver>::register_observer(*this);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, [&scene](){
      for (Tag* tag : scene.selected_tags()) {
        tag->deselect();
      }
      for (Style* style : scene.selected_styles()) {
        style->deselect();
      }
    });
  }
}

StyleListAdapter* StyleListView::model() const
{
  return static_cast<StyleListAdapter*>(QListView::model());
}

void StyleListView::contextMenuEvent(QContextMenuEvent *event)
{
  auto menu = std::make_unique<QMenu>();

  action(*menu, tr("&new"), [this](){
    m_scene.submit<NewStyleCommand>(m_scene, std::make_unique<Style>());
  });

  menu->move(event->globalPos());
  menu->show();

  menu->setAttribute(Qt::WA_DeleteOnClose);
  menu.release();
}

void StyleListView::mouseReleaseEvent(QMouseEvent *event)
{
  QListView::mouseReleaseEvent(event);
  Q_EMIT mouse_released();
}

void StyleListView::set_selection(const SetOfPropertyOwner& selection)
{
  for (size_t i = 0; i < model()->rowCount(); ++i) {
    assert(m_scene.style(i).is_selected() == selection.count(&m_scene.style(i)));
    QModelIndex index = model()->index(i, 0, QModelIndex());
    if (m_scene.style(i).is_selected()) {
      selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Select);
    } else {
      selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Deselect);
    }
  }
};

}  // namespace
