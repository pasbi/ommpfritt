#pragma once

#include "managers/manager.h"

class QItemSelection;
class QItemSelection;

namespace omm
{

template<typename ItemViewT>
class ItemManager : public Manager
{
protected:
  using item_model_type = typename ItemViewT::model_type;
  explicit ItemManager(const QString& title, Scene& scene, item_model_type& model);
  // bool eventFilter(QObject* object, QEvent* event) override;

private:
  // virtual void on_selection_changed( const QItemSelection& selection,
  //                                    const QItemSelection& old_selection );
};

}  // namespace omm
