#include "preferences/preferencestreeview.h"
#include "preferences/preferencestreeviewdelegate.h"
#include <QPushButton>

namespace omm
{

class UiColorsTreeViewDelegate : public SettingsTreeViewDelegate<QPushButton>
{
protected:
  void set_editor_data(QPushButton &editor, const QModelIndex &index) const override;
  bool set_model_data(QPushButton &editor, QAbstractItemModel &model, const QModelIndex &index) const override;
};

}  // namespace omm
