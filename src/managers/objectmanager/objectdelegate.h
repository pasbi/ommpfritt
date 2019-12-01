#pragma once

#include <QStyledItemDelegate>
#include "managers/objectmanager/objecttreeselectionmodel.h"

namespace omm
{

class ObjectTreeView;
class ProxyChain;

class ObjectDelegate : public QStyledItemDelegate
{
public:
  ObjectDelegate(ObjectTreeView& object_tree_view, ObjectTreeSelectionModel& selection_model, ProxyChain& proxy_chain);
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  QModelIndexList tmp_selection;
  QItemSelectionModel::SelectionFlag selection_flag = QItemSelectionModel::Select;

private:
  ObjectTreeView& m_object_tree_view;
  ObjectTreeSelectionModel& m_selection_model;
  ProxyChain& m_proxy_chain;
};

}  // namespace omm
