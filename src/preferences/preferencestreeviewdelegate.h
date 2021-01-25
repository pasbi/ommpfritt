#pragma once

#include <QItemDelegate>
#include <memory>

namespace omm
{
class AbstractPreferencesTreeViewDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  explicit AbstractPreferencesTreeViewDelegate(QAbstractItemView& view);
  void transfer_editor_data_to_model();
  void set_model(QAbstractItemModel& model);

public:
  void close_current_editor();

protected:
  QAbstractItemModel* m_model{};
  mutable QWidget* m_current_editor = nullptr;
  mutable QModelIndex m_current_editor_index;
  QAbstractItemView& m_view;
};

template<typename EditorT>
class PreferencesTreeViewDelegate : public AbstractPreferencesTreeViewDelegate
{
public:
  using AbstractPreferencesTreeViewDelegate::AbstractPreferencesTreeViewDelegate;

protected:
  QWidget* createEditor(QWidget* parent,
                        const QStyleOptionViewItem&,
                        const QModelIndex& index) const override
  {
    auto editor = std::make_unique<EditorT>(parent);
    m_current_editor = editor.get();
    m_current_editor_index = index;
    QObject::connect(editor.get(), &QWidget::destroyed, this, [this]() {
      m_current_editor = nullptr;
    });
    return editor.release();
  };

  virtual void set_editor_data(EditorT& editor, const QModelIndex& index) const = 0;
  virtual bool
  set_model_data(EditorT& editor, QAbstractItemModel& model, const QModelIndex& index) const = 0;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override
  {
    set_editor_data(*static_cast<EditorT*>(editor), index);
  }

  void
  setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
  {
    const bool s = set_model_data(*static_cast<EditorT*>(editor), *model, index);
    Q_UNUSED(s)
    assert(s);
  }
};

}  // namespace omm
