#include "preferences/preferencestreeviewdelegate.h"
#include "logging.h"
#include <QAbstractItemView>

namespace omm
{
AbstractPreferencesTreeViewDelegate::AbstractPreferencesTreeViewDelegate(QAbstractItemView& view)
    : m_view(view)
{
}

void AbstractPreferencesTreeViewDelegate::transfer_editor_data_to_model()
{
  if (m_current_editor != nullptr && m_current_editor_index.isValid()) {
    setModelData(m_current_editor, m_model, m_current_editor_index);
  }
}

void AbstractPreferencesTreeViewDelegate::set_model(QAbstractItemModel& model)
{
  m_model = &model;
}

void AbstractPreferencesTreeViewDelegate::close_current_editor()
{
  m_view.closePersistentEditor(m_current_editor_index);
  m_current_editor = nullptr;
  m_current_editor_index = QModelIndex();
}

}  // namespace omm
