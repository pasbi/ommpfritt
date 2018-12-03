#pragma once

#include "scene/abstractselectionobserver.h"

class QMenu;
class QModelIndex;
class QContextMenuEvent;

namespace omm
{

class Style;

template<typename ItemViewT, typename ItemModelT>
class ManagerItemView : public ItemViewT, public AbstractSelectionObserver
{
public:
  explicit ManagerItemView();
  virtual ~ManagerItemView();
  void set_model(ItemModelT* model);
  ItemModelT* model() const;

  template<typename RemoveCommand, typename PropertyOwner=typename ItemModelT::item_type>
  bool remove_selection(const std::set<PropertyOwner*>& selection);

protected:
  virtual void populate_menu(QMenu& menu, const QModelIndex& index) const = 0;
  void contextMenuEvent(QContextMenuEvent *event);
};

}  // namespace omm
