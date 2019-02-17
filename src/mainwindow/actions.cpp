#include "mainwindow/actions.h"
#include "scene/scene.h"
#include "commands/modifypointscommand.h"
#include "commands/propertycommand.h"
#include "properties/optionsproperty.h"
#include "tags/scripttag.h"
#include "mainwindow/application.h"

namespace
{

omm::Scene& scene() { return omm::Application::instance().scene; }

void modify_tangents(omm::Path::InterpolationMode mode)
{
  const auto paths = omm::Object::cast<omm::Path>(scene().object_selection());
  std::map<omm::Path*, std::map<omm::Point*, omm::Point>> map;
  for (omm::Path* path : paths) {
    map[path] = path->modified_points(true, mode);
  }

  constexpr auto bezier_mode = static_cast<std::size_t>(omm::Path::InterpolationMode::Bezier);
  const auto interpolation_properties = ::transform<omm::Property*>(paths, [](omm::Path* path) {
    return &path->property(omm::Path::INTERPOLATION_PROPERTY_KEY);
  });

  if (map.size() > 0) {
    scene().undo_stack.beginMacro("modify tangents");
    using OptionsPropertyCommand = omm::PropertiesCommand<omm::OptionsProperty>;
    scene().submit<OptionsPropertyCommand>(interpolation_properties, bezier_mode);
    scene().submit<omm::ModifyPointsCommand>(map);
    scene().undo_stack.endMacro();
  }
}

}  // namespace

namespace omm::actions
{


void make_linear() { modify_tangents(Path::InterpolationMode::Linear); }
void make_smooth() { modify_tangents(Path::InterpolationMode::Smooth); }

void remove_selected_points()
{
  std::map<Path*, std::vector<std::size_t>> map;
  for (auto* path : Object::cast<Path>(scene().object_selection())) {
    map[path] = path->selected_points();
  }

  scene().submit<RemovePointsCommand>(map);
}

void subdivide()
{
  const std::size_t n = 1;
  const auto subdivide = [n]( const auto& points, const std::size_t i,
                              auto& sequences, std::size_t& accu ) {
    const Point& a = *points[i];
    const Point& b = *points[(i+1)%points.size()];
    if (a.is_selected && b.is_selected) {
      std::list<Point> intermediates;
      Cubic segment(a, b);
      for (std::size_t j = 0; j < n; ++j) {
        const double t = double(j+1) / double(n+1);
        intermediates.push_back(Point(segment.evaluate(t), 1.0));
      }
      sequences.push_back(Path::PointSequence{ i+1+accu, intermediates });
      accu += intermediates.size();
    }
  };
  const auto make_subdivision_sequences = [subdivide](Path& path) {
    std::size_t accu = 0;
    std::list<Path::PointSequence> sequences;
    for (std::size_t i = 0; i < path.points().size() - 1; ++i) {
      subdivide(path.points(), i, sequences, accu);
    }
    if (path.is_closed()) {
      subdivide(path.points(), path.points().size() - 1, sequences, accu);
    }
    return std::vector(sequences.begin(), sequences.end());
  };
  std::map<Path*, std::vector<Path::PointSequence>> map;
  for (auto* path : Object::cast<Path>(scene().object_selection())) {
    map[path] = make_subdivision_sequences(*path);
  }

  scene().submit<AddPointsCommand>(map);
}

void evaluate()
{
  for (Tag* tag : scene().tags()) {
    auto* script_tag = type_cast<ScriptTag*>(tag);
    if (script_tag != nullptr) { script_tag->force_evaluate(); }
  }
}

}  // namespace omm::actions