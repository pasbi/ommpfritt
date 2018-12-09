#pragma once

#include <QListView>
#include "common.h"
#include "managers/manageritemview.h"
#include "managers/stylemanager/stylelistadapter.h"

namespace omm
{

class Object;

class StyleListView : public ManagerItemView<QListView, StyleListAdapter>
{
public:
  using ManagerItemView<QListView, StyleListAdapter>::ManagerItemView;
  void set_selection(const SetOfPropertyOwner& selection) override;
  AbstractPropertyOwner::Kind displayed_kinds() const override;

protected:
  void populate_menu(QMenu& menu, const QModelIndex& index) const override;
};

}  // namespace
