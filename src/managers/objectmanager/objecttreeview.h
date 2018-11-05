#pragma once

#include <QTreeView>
#include "common.h"

namespace omm
{

class Object;
class ObjectTreeAdapter;

class ObjectTreeView : public QTreeView
{
public:
  explicit ObjectTreeView();
  void set_model(ObjectTreeAdapter& model);
  ObjectTreeAdapter& model() const;
  void remove_selected() const;
  ObjectRefs selection() const;

protected:
  void contextMenuEvent(QContextMenuEvent *event);
  void populate_menu(QMenu& menu, const Object& subject) const;

};

}  // namespace
