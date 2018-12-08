#include "managers/stylemanager/stylelistadapter.h"

#include "menuhelper.h"
#include "managers/stylemanager/stylelistview.h"
#include "commands/newstylecommand.h"

namespace omm
{

void StyleListView::populate_menu(QMenu& menu, const QModelIndex& index) const
{
  Scene& scene = model()->scene();
  action(menu, tr("&new"), [&scene](){
    scene.submit<NewStyleCommand>(scene.styles, scene.default_style().copy());
  });

  if (index.isValid()) {
    action(menu, tr("&remove"), *this, &ManagerItemView::remove_selection);
  }

}

void StyleListView::set_selection(const SetOfPropertyOwner& selection)
{
  // TODO
  // auto& scene = this->model()->scene();
  // for (size_t i = 0; i < model()->rowCount(); ++i) {
  //   assert(scene.style(i).is_selected() == selection.count(&scene.style(i)));
  //   QModelIndex index = model()->index(i, 0, QModelIndex());
  //   if (scene.style(i).is_selected()) {
  //     selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Select);
  //   } else {
  //     selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Deselect);
  //   }
  // }
}

}  // namespace
