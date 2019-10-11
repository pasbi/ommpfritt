#include "preferences/preferencestreeview.h"
#include <QHeaderView>
#include "keybindings/keybindings.h"
#include "logging.h"
#include "widgets/keysequenceedit.h"
#include "preferences/preferencestreeviewdelegate.h"

namespace omm
{

PreferencesTreeView::PreferencesTreeView(QWidget* parent)
  : QTreeView(parent)
{
  header()->hide();
  for (int i = 0; i < 3; ++i) {
    header()->setSectionResizeMode(QHeaderView::Fixed);
  }
  connect(this, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(update_column_width()));
  connect(this, SIGNAL(expanded(const QModelIndex&)), this, SLOT(update_column_width()));
}

PreferencesTreeView::~PreferencesTreeView()
{
}

void PreferencesTreeView::set_model(QAbstractItemModel& model)
{
  assert(this->model() == nullptr);
  setModel(&model);
  m_sequence_column_delegate->set_model(model);
  setItemDelegateForColumn(1, m_sequence_column_delegate.get());
  setSelectionMode(QAbstractItemView::NoSelection);
}

void PreferencesTreeView::transfer_editor_data_to_model()
{
  m_sequence_column_delegate->transfer_editor_data_to_model();
}

void PreferencesTreeView::set_delegate(std::unique_ptr<AbstractPreferencesTreeViewDelegate> delegate)
{
  assert(m_sequence_column_delegate == nullptr);
  assert(delegate != nullptr);
  m_sequence_column_delegate = std::move(delegate);
}

void PreferencesTreeView::resizeEvent(QResizeEvent* event)
{
  update_column_width();
  QTreeView::resizeEvent(event);
}

void PreferencesTreeView::update_column_width()
{
  const int required_width_name = sizeHintForColumn(0);
  const int required_width_sequence = sizeHintForColumn(1);
  const int available_width = viewport()->width();
  static constexpr int max_width_sequence = 350;

  const int excess = std::max(0, available_width - required_width_name - required_width_sequence);
  const int width_sequence = std::max(required_width_sequence,
                                  std::min(max_width_sequence, required_width_sequence + excess));
  const int width_name = std::max(available_width - width_sequence, required_width_name);

  setColumnWidth(0, width_name);
  setColumnWidth(1, width_sequence);
}

}  // namespace omm
