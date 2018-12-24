#pragma once

#include <QListView>
#include "common.h"
#include "managers/manageritemview.h"
#include "scene/listadapter.h"

namespace omm
{

class Object;

class StyleListView : public ManagerItemView<QListView, ListAdapter<Style>>
{
public:
  using model_type = ListAdapter<Style>;
  using ManagerItemView<QListView, model_type>::ManagerItemView;

protected:
  void populate_menu(QMenu& menu, const QModelIndex& index) const override;
};

}  // namespace
