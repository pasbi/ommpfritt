#pragma once

#include <QTreeView>
#include "common.h"

namespace omm
{

class Object;
class ObjectTreeAdapter;

class ObjectTreeView : public QTreeView
{
  Q_OBJECT
public:
  explicit ObjectTreeView();
  void set_model(ObjectTreeAdapter& model);
  ObjectTreeAdapter& model() const;

Q_SIGNALS:
  void mouse_released();

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void populate_menu(QMenu& menu, const Object& subject) const;

private:
  void remove_selected() const;
  void attach_tag_to_selected(const std::string& tag_class) const;

};

}  // namespace
