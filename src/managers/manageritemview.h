#pragma once

#include "aspects/propertyowner.h"

class QMenu;
class QModelIndex;
class QContextMenuEvent;
class QMouseEvent;
class QWidget;

namespace omm
{

class Style;

template<typename ItemViewT, typename ItemModelT>
class ManagerItemView : public ItemViewT
{
public:
  using item_type = typename ItemModelT::item_type;
  explicit ManagerItemView(ItemModelT& model);
  virtual ~ManagerItemView();
  ItemModelT* model() const;
  virtual std::set<AbstractPropertyOwner*> selected_items() const;

  virtual bool remove_selection();

protected:
  virtual void populate_menu(QMenu& menu, const QModelIndex& index) const = 0;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void mouseReleaseEvent(QMouseEvent* e) override;

private:
  using ItemViewT::setModel;
};

bool can_remove_selection(QWidget* parent, Scene& scene,
                          const std::set<AbstractPropertyOwner*>& selection,
                          std::set<ReferenceProperty*>& properties);

}  // namespace omm
