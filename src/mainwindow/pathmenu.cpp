#include "mainwindow/pathmenu.h"
#include "menuhelper.h"
#include "objects/path.h"
#include "scene/scene.h"
#include "commands/modifypointscommand.h"
#include "commands/propertycommand.h"
#include "properties/optionsproperty.h"
#include "mainwindow/actions.h"

namespace omm
{

std::vector<Point> copy_points(const std::vector<Point*>& points)
{
  return ::transform<Point>(points, [](Point* p) { return *p; });
}

PathMenu::PathMenu(Scene& scene, QWidget* parent)
  : QMenu(parent)
{
  setTitle(tr("&Path"));
  action(*this, tr("make smooth"), [&scene]() {
    actions::make_smooth(scene);
  });
  action(*this, tr("make linear"), [&scene]() {
    actions::make_linear(scene);
  });
  action(*this, tr("remove selected points"), [&scene]() {
    actions::remove_selected_points(scene);
  });
  action(*this, tr("subdivide"), [&scene]() {
    actions::subdivide(scene);
  });
}

}  // namespace
