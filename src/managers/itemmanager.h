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
  
  ItemModelT m_item_model;
};

}  // namespace omm
