#include "managers/stylemanager/stylelistadapter.h"

#include <QMenu>
#include <QContextMenuEvent>
#include "menuhelper.h"
#include "managers/stylemanager/stylelistview.h"
#include "commands/newstylecommand.h"

namespace omm
{

StyleListView::StyleListView(Scene& scene)
  : m_scene(scene)
{

}

StyleListView::~StyleListView()
{

}

void StyleListView::set_model(StyleListAdapter* model)
{
  setModel(model);
}

StyleListAdapter* StyleListView::model() const
{
  return static_cast<StyleListAdapter*>(QListView::model());
}

void StyleListView::contextMenuEvent(QContextMenuEvent *event)
{
  auto menu = std::make_unique<QMenu>();

  action(*menu, tr("&new"), [this](){
    m_scene.submit<NewStyleCommand>(m_scene, std::make_unique<Style>());
  });

  menu->move(event->globalPos());
  menu->show();

  menu->setAttribute(Qt::WA_DeleteOnClose);
  menu.release();
}

}  // namespace
