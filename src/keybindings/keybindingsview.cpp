#include "keybindings/keybindingsview.h"
#include <QHeaderView>
#include "keybindings/keybindings.h"
#include "logging.h"
#include "widgets/keysequenceedit.h"

namespace omm
{

QWidget* KeySequenceItemDelegate::createEditor( QWidget* parent,
                                                const QStyleOptionViewItem&,
                                                const QModelIndex& index) const
{
  auto editor = std::make_unique<KeySequenceEdit>(parent);
  m_current_editor = editor.get();
  m_current_editor_index = index;
  QObject::connect(editor.get(), &QWidget::destroyed, this, [this]() {
    m_current_editor = nullptr;
  });
  return editor.release();
}

void KeySequenceItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  auto* sequence_edit = static_cast<KeySequenceEdit*>(editor);
  sequence_edit->set_default_key_sequence(index.data(KeyBindings::DEFAULT_KEY_SEQUENCE_ROLE)
                                          .value<QKeySequence>());
  sequence_edit->set_key_sequence(index.data(Qt::EditRole).value<QKeySequence>());
}

void KeySequenceItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
                                            const QModelIndex &index ) const
{
  auto* sequence_edit =static_cast<KeySequenceEdit*>(editor);
  const bool s = model->setData(index, sequence_edit->key_sequence(), Qt::EditRole);
  assert(s);
}

void KeySequenceItemDelegate::transfer_editor_data_to_model()
{
  if (m_current_editor != nullptr && m_current_editor_index.isValid()) {
    setModelData(m_current_editor, m_model, m_current_editor_index);
  }
}

void KeySequenceItemDelegate::set_model(QAbstractItemModel* model)
{
  m_model = model;
}

KeyBindingsView::KeyBindingsView(QWidget* parent)
  : QTreeView(parent)
  , m_sequence_column_delegate(std::make_unique<KeySequenceItemDelegate>())
{
  header()->hide();
  for (int i = 0; i < 3; ++i) {
    header()->setSectionResizeMode(QHeaderView::Fixed);
  }
  connect(this, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(update_column_width()));
  connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(update_column_width()));
}

void KeyBindingsView::set_model(QAbstractItemModel& model)
{
  assert(this->model() == nullptr);
  setModel(&model);
  m_sequence_column_delegate->set_model(&model);
  setItemDelegateForColumn(1, m_sequence_column_delegate.get());
  setSelectionMode(QAbstractItemView::NoSelection);
}

void KeyBindingsView::transfer_editor_data_to_model()
{
  m_sequence_column_delegate->transfer_editor_data_to_model();
}

void KeyBindingsView::resizeEvent(QResizeEvent* event)
{
  update_column_width();
  QTreeView::resizeEvent(event);
}

void KeyBindingsView::update_column_width()
{
  const int required_width_name = sizeHintForColumn(0);
  const int required_width_sequence = sizeHintForColumn(1);
  const int available_width = viewport()->width();
  static constexpr int max_width_sequence = 250;

  const int excess = std::max(0, available_width - required_width_name - required_width_sequence);
  const int width_sequence = std::max(required_width_sequence,
                                  std::min(max_width_sequence, required_width_sequence + excess));
  const int width_name = std::max(available_width - width_sequence, required_width_name);

  setColumnWidth(0, width_name);
  setColumnWidth(1, width_sequence);
}

}  // namespace omm
