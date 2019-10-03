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
  void transfer_editor_data_to_model();
  void set_model(QAbstractItemModel* model);
private:
  mutable QModelIndex m_current_editor_index;
  mutable QWidget* m_current_editor;
  QAbstractItemModel* m_model = nullptr;
};

class KeyBindingsView : public QTreeView
{
  Q_OBJECT
public:
  explicit KeyBindingsView(QWidget* parent = nullptr);
  void set_model(QAbstractItemModel& key_bindings);
  void transfer_editor_data_to_model();

protected:
  void resizeEvent(QResizeEvent* event) override;
private:
  std::unique_ptr<KeySequenceItemDelegate> m_sequence_column_delegate;

private Q_SLOTS:
  void update_column_width();
};

}  // namespace omm
