#include "preferences/preferencestreeviewdelegate.h"

namespace omm
{


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


}  // namespace omm
