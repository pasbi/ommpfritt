#include "mainwindow/pathmenu.h"
#include "menuhelper.h"
#include "objects/path.h"
#include "scene/scene.h"
#include "commands/modifypointscommand.h"
#include "commands/propertycommand.h"
#include "properties/optionsproperty.h"

namespace
{

void modify_tangents(omm::Scene& scene, omm::Path::InterpolationMode mode)
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

  if (map.size() > 0) {
    scene.undo_stack.beginMacro("modify tangents");
    using OptionsPropertyCommand = omm::PropertiesCommand<omm::OptionsProperty>;
    scene.submit<OptionsPropertyCommand>(interpolation_properties, bezier_mode);
    scene.submit<omm::ModifyPointsCommand>(map);
    scene.undo_stack.endMacro();
  }
}

void remove_selected_points(omm::Scene& scene)
{
  std::map<omm::Path*, std::vector<std::size_t>> map;
  for (auto* path : omm::Object::cast<omm::Path>(scene.object_selection())) {
    map[path] = path->selected_points();
  }

  scene.submit<omm::RemovePointsCommand>(map);
}

void subdivide(omm::Scene& scene)
{
  const std::size_t n = 1;
  const auto subdivide = [n]( const auto& points, const std::size_t i,
                              auto& sequences, std::size_t& accu ) {
    const omm::Point& a = *points[i];
    const omm::Point& b = *points[(i+1)%points.size()];
    if (a.is_selected && b.is_selected) {
      std::list<omm::Point> intermediates;
      omm::Cubic segment(a, b);
      for (std::size_t j = 0; j < n; ++j) {
        const double t = double(j+1) / double(n+1);
        intermediates.push_back(omm::Point(segment.evaluate(t), 1.0));
      }
      sequences.push_back(omm::Path::PointSequence{ i+1+accu, intermediates });
      accu += intermediates.size();
    }
  };
  const auto make_subdivision_sequences = [subdivide](omm::Path& path) {
    std::size_t accu = 0;
    std::list<omm::Path::PointSequence> sequences;
    for (std::size_t i = 0; i < path.points().size() - 1; ++i) {
      subdivide(path.points(), i, sequences, accu);
    }
    if (path.is_closed()) {
      subdivide(path.points(), path.points().size() - 1, sequences, accu);
    }
    return std::vector(sequences.begin(), sequences.end());
  };
  std::map<omm::Path*, std::vector<omm::Path::PointSequence>> map;
  for (auto* path : omm::Object::cast<omm::Path>(scene.object_selection())) {
    map[path] = make_subdivision_sequences(*path);
  }

  scene.submit<omm::AddPointsCommand>(map);
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
  action(*this, tr("make smooth"), [&scene]() {
    modify_tangents(scene, Path::InterpolationMode::Smooth);
  });
  action(*this, tr("make linear"), [&scene]() {
    modify_tangents(scene, Path::InterpolationMode::Linear);
  });
  action(*this, tr("remove selected points"), [&scene]() {
    remove_selected_points(scene);
  });
  action(*this, tr("subdivide"), [&scene]() {
    subdivide(scene);
  });
}

}  // namespace
