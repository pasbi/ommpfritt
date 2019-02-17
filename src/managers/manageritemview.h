#pragma once

#include "aspects/propertyowner.h"

class QMenu;
class QModelIndex;
class QContextMenuEvent;
class QMouseEvent;
class QWidget;
class QItemSelectionModel;

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
  void setSelectionModel(QItemSelectionModel* model) override;

protected:
  virtual void populate_menu(QMenu& menu, const QModelIndex& index) const = 0;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;

private:
  using ItemViewT::setModel;
  bool m_block_selection_change_signal = false;
};

bool can_remove_selection(QWidget* parent, Scene& scene,
                          const std::set<AbstractPropertyOwner*>& selection,
                          std::set<Property*>& properties);

}  // namespace omm
