#pragma once

#include <QTreeView>

namespace omm
{
class ToolBarItemModel;

class ToolBarItemView : public QTreeView
{
public:
  using QTreeView::QTreeView;
  void setModel(QAbstractItemModel* model) override;

protected:
  //  void startDrag(Qt::DropActions supported_action) override;
  void dropEvent(QDropEvent* event) override;
};

}  // namespace omm
