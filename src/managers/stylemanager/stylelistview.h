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
  Q_OBJECT
public:
  using model_type = ListAdapter<Style>;
  using ManagerItemView<QListView, model_type>::ManagerItemView;

public Q_SLOTS:
  void set_selection(const std::set<Style*>& selection);
};

}  // namespace
