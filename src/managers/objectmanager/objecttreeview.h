#pragma once

#include <QTreeView>
#include "common.h"
#include "managers/objectmanager/tagsitemdelegate.h"
#include "scene/abstractselectionobserver.h"

namespace omm
{

class Object;
class ObjectTreeAdapter;

class ObjectTreeView : public QTreeView, public AbstractSelectionObserver
{
  Q_OBJECT
public:
  explicit ObjectTreeView();
  ~ObjectTreeView();
  void set_model(ObjectTreeAdapter* model);
  ObjectTreeAdapter* model() const;
  void set_selection(const std::set<AbstractPropertyOwner*>& selection) override;

Q_SIGNALS:
  void mouse_released();

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void populate_menu(QMenu& menu, const Object& subject) const;

private:
  void remove_selected() const;
  void attach_tag_to_selected(const std::string& tag_class) const;
  std::unique_ptr<TagsItemDelegate> m_tags_item_delegate;
  void set_selection(const std::set<AbstractPropertyOwner*>& selection, Object& root);

};

}  // namespace
