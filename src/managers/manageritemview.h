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
  void setSelectionModel(QItemSelectionModel* model) override;

protected:
  void mouseReleaseEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;

private:
  using ItemViewT::setModel;
  bool m_block_selection_change_signal = false;
};

}  // namespace omm
