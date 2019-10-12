#include "preferences/preferencestreeview.h"
#include "preferences/preferencestreeviewdelegate.h"
#include "preferences/uicoloredit.h"

namespace omm
{

class UiColorsTreeViewDelegate : public PreferencesTreeViewDelegate<UiColorEdit>
{
protected:
  void set_editor_data(UiColorEdit &editor, const QModelIndex &index) const override;
  bool set_model_data(UiColorEdit &editor, QAbstractItemModel &model, const QModelIndex &index) const override;
};

}  // namespace omm
