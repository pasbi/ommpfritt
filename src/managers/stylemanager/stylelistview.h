#pragma once

#include <QListView>
#include "common.h"

namespace omm
{

class Object;
class StyleListAdapter;

class StyleListView : public QListView
{
  Q_OBJECT
public:
  explicit StyleListView(Scene& scene);
  ~StyleListView();
  void set_model(StyleListAdapter* model);
  StyleListAdapter* model() const;

protected:
  void contextMenuEvent(QContextMenuEvent *event);

private:
  Scene& m_scene;
};

}  // namespace
