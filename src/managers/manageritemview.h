#pragma once

#include "scene/abstractselectionobserver.h"

class QMenu;
class QModelIndex;
class QContextMenuEvent;
class QMouseEvent;

namespace omm
{

class Style;

template<typename ItemViewT, typename ItemModelT>
class ManagerItemView : public ItemViewT, public AbstractSelectionObserver
{
public:
  using item_type = typename ItemModelT::item_type;
  explicit ManagerItemView();
  virtual ~ManagerItemView();
  void set_model(ItemModelT* model);
  ItemModelT* model() const;

  template<typename RemoveCommand, typename StructureT>
  bool remove_selection(const std::set<item_type*>& selection);

protected:
  virtual void populate_menu(QMenu& menu, const QModelIndex& index) const = 0;
  void contextMenuEvent(QContextMenuEvent *event);
  void mouseReleaseEvent(QMouseEvent* e);
};

}  // namespace omm
