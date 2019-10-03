#pragma once

#include <QTreeView>
#include <QItemDelegate>
#include <QKeySequenceEdit>
#include <memory>

namespace omm
{

class KeyBindings;

class KeySequenceItemDelegate : public QItemDelegate
{
public:
  QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& option,
                         const QModelIndex& index ) const override;
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void setModelData( QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index ) const override;
};

class KeyBindingsView : public QTreeView
{
public:
  explicit KeyBindingsView(QAbstractItemModel& key_bindings);

private:
  std::unique_ptr<KeySequenceItemDelegate> m_sequence_column_delegate;
};

}  // namespace omm
