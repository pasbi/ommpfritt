#pragma once

#include "managers/manager.h"

class QItemSelection;
class QItemSelection;

namespace omm
{

template<typename ItemViewT, typename ItemModelT>
class ItemManager : public Manager
{
protected:
  explicit ItemManager(const QString& title, Scene& scene);
  bool eventFilter(QObject* object, QEvent* event) override;

private:
  virtual void on_selection_changed( const QItemSelection& selection,
                                     const QItemSelection& old_selection );

  // order matters as m_item_view uses m_item_model.
  ItemModelT m_item_model;
  ItemViewT m_item_view;      // must be created before and destroyed after `m_item_model`

};

}  // namespace omm
