#pragma once

#include <QItemDelegate>
#include <memory>

namespace omm
{

class AbstractPreferencesTreeViewDelegate : public QItemDelegate
{
public:
  void transfer_editor_data_to_model();
  void set_model(QAbstractItemModel& model);

protected:
  QAbstractItemModel* m_model;
  mutable QWidget* m_current_editor = nullptr;
  mutable QModelIndex m_current_editor_index;
};

template<typename EditorT>
class SettingsTreeViewDelegate : public AbstractPreferencesTreeViewDelegate
{
protected:
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const override
  {
    auto editor = std::make_unique<EditorT>(parent);
    m_current_editor = editor.get();
    m_current_editor_index = index;
    QObject::connect(editor.get(), &QWidget::destroyed, this, [this]() {
      m_current_editor = nullptr;
    });
    return editor.release();
  };

  virtual void set_editor_data(EditorT& editor, const QModelIndex &index) const = 0;
  virtual bool set_model_data(EditorT& editor, QAbstractItemModel& model, const QModelIndex& index) const = 0;

  void setEditorData(QWidget *editor, const QModelIndex &index) const
  {
    set_editor_data(*static_cast<EditorT*>(editor), index);
  }

  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
  {
    const bool s = set_model_data(*static_cast<EditorT*>(editor), *model, index);
    assert(s);
  }
};


}  // namespace
