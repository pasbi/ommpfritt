#pragma once

#include <QListView>
#include "common.h"
#include "scene/abstractstylelistobserver.h"

namespace omm
{

class Object;
class StyleListAdapter;

class StyleListView : public QListView, public AbstractSelectionObserver
{
  Q_OBJECT
public:
  explicit StyleListView(Scene& scene);
  ~StyleListView();
  void set_model(StyleListAdapter* model);
  StyleListAdapter* model() const;
  void set_selection(const SetOfPropertyOwner& selection) override;

Q_SIGNALS:
  void mouse_released();

protected:
  void contextMenuEvent(QContextMenuEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);


private:
  Scene& m_scene;
};

}  // namespace
