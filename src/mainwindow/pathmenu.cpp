#include "mainwindow/pathmenu.h"
#include "menuhelper.h"
#include "objects/path.h"
#include "scene/scene.h"
#include "commands/modifypointscommand.h"
#include "commands/propertycommand.h"

namespace
{

auto modified_tangents(omm::Scene& scene, omm::Path::InterpolationMode mode)
{
  const auto paths = omm::Object::cast<omm::Path>(scene.object_selection());
  std::map<omm::Path*, std::map<omm::Point*, omm::Point>> map;
  for (omm::Path* path : paths) {
    map[path] = path->modified_points(true, mode);
  }

  constexpr auto bezier_mode = static_cast<std::size_t>(omm::Path::InterpolationMode::Bezier);
  const auto interpolation_properties = ::transform<omm::Property*>(paths, [](omm::Path* path) {
    return &path->property(omm::Path::INTERPOLATION_PROPERTY_KEY);
  });

  scene.undo_stack.beginMacro("modify tangents");
  scene.submit<omm::PropertiesCommand<std::size_t>>(interpolation_properties, bezier_mode);
  scene.submit<omm::ModifyPointsCommand>(map);
  scene.undo_stack.endMacro();
}

}  // namespace

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
  action(*this, tr("make smooth"), [this, &scene]() {
    modified_tangents(scene, Path::InterpolationMode::Smooth);
  });
  action(*this, tr("make linear"), [this, &scene]() {
    modified_tangents(scene, Path::InterpolationMode::Linear);
  });
}

}  // namespace