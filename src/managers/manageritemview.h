#pragma once

#include "aspects/propertyowner.h"

class QMenu;
class QModelIndex;
class QContextMenuEvent;
class QMouseEvent;
class QWidget;
class QItemSelectionModel;
class QKeyEvent;
class QFocusEvent;

namespace omm
{

class Style;

template<typename ItemViewT, typename ItemModelT>
class ManagerItemView : public ItemViewT
{
public:
  using item_type = typename ItemModelT::item_type;
  explicit ManagerItemView(ItemModelT& model);
  virtual ~ManagerItemView() = default;
  ItemModelT* model() const;
  virtual std::set<AbstractPropertyOwner*> selected_items() const;

protected:
  void mouseReleaseEvent(QMouseEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void keyPressEvent(QKeyEvent* e) override;
  void keyReleaseEvent(QKeyEvent* e) override;
  void focusInEvent(QFocusEvent* e) override;

private:
  using ItemViewT::setModel;
  bool m_block_selection_change_signal = false;
};

}  // namespace omm
