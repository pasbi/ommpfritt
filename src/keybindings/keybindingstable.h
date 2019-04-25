#pragma once

#include <QTableView>
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

class KeyBindingsTable : public QTableView
{
public:
  explicit KeyBindingsTable(QAbstractItemModel& key_bindings);

private:
  std::unique_ptr<KeySequenceItemDelegate> m_sequence_column_delegate;
};

}  // namespace omm
