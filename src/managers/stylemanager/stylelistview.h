#pragma once

#include <QListView>
#include "common.h"
#include "managers/manageritemview.h"
#include "scene/stylelist.h"

namespace omm
{

class Object;
class StyleList;
class StyleListView : public ManagerItemView<QListView, StyleList>
{
  Q_OBJECT
public:
  using model_type = StyleList;
  using ManagerItemView<QListView, model_type>::ManagerItemView;

public Q_SLOTS:
  void set_selection(const std::set<Style*>& selection);
};

}  // namespace
