#include "managers/stylemanager/stylelistview.h"

#include "menuhelper.h"
#include "commands/addcommand.h"
#include "scene/scene.h"

namespace omm
{

void StyleListView::populate_menu(QMenu& menu, const QModelIndex& index) const
{
  Scene& scene = model()->scene();
  action(menu, tr("&new"), [&scene](){
    using command_type = AddCommand<List<Style>>;
    scene.submit<command_type>(scene.styles, scene.default_style().copy());
  });

  if (index.isValid()) {
    action(menu, tr("&remove"), *this, &ManagerItemView::remove_selection);
  }
}

// void StyleListView::set_selection(const SetOfPropertyOwner& selection)
// {
//   auto& scene = this->model()->scene();
//   for (size_t i = 0; i < model()->rowCount(); ++i) {
//     assert(scene.styles.item(i).is_selected() == selection.count(&scene.styles.item(i)));
//     QModelIndex index = model()->index(i, 0, QModelIndex());
//     if (scene.styles.item(i).is_selected()) {
//       selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Select);
//     } else {
//       selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::Deselect);
//     }
//   }
// }

AbstractPropertyOwner::Kind StyleListView::displayed_kinds() const
{
  return AbstractPropertyOwner::Kind::Style;
}

}  // namespace
