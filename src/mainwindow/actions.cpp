#include "mainwindow/actions.h"
#include "scene/scene.h"
#include "commands/modifypointscommand.h"
#include "commands/propertycommand.h"
#include "commands/addcommand.h"
#include "commands/removecommand.h"
#include "properties/optionsproperty.h"
#include "tags/scripttag.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include <QUndoStack>
#include "common.h"
#include "properties/referenceproperty.h"

namespace
{

void modify_tangents(omm::Path::InterpolationMode mode, omm::Application& app)
{
  const auto paths = omm::Object::cast<omm::Path>(app.scene.item_selection<omm::Object>());
  std::map<omm::Path*, std::map<omm::Point*, omm::Point>> map;
  for (omm::Path* path : paths) {
    map[path] = path->modified_points(true, mode);
  }

  constexpr auto bezier_mode = static_cast<std::size_t>(omm::Path::InterpolationMode::Bezier);
  const auto interpolation_properties = ::transform<omm::Property*>(paths, [](omm::Path* path) {
    return &path->property(omm::Path::INTERPOLATION_PROPERTY_KEY);
  });

  if (map.size() > 0) {
    app.scene.undo_stack.beginMacro(QObject::tr("modify tangents"));
    using OptionsPropertyCommand = omm::PropertiesCommand<omm::OptionsProperty>;
    app.scene.submit<OptionsPropertyCommand>(interpolation_properties, bezier_mode);
    app.scene.submit<omm::ModifyPointsCommand>(map);
    app.scene.undo_stack.endMacro();
  }
}

}  // namespace

namespace omm::actions
{

void make_linear(Application& app) { modify_tangents(Path::InterpolationMode::Linear, app); }
void make_smooth(Application& app) { modify_tangents(Path::InterpolationMode::Smooth, app); }

void remove_selected_points(Application& app)
{
  std::map<Path*, std::vector<std::size_t>> map;
  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    map[path] = path->selected_points();
  }

  app.scene.submit<RemovePointsCommand>(map);
}

void subdivide(Application& app)
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
  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    map[path] = make_subdivision_sequences(*path);
  }

  app.scene.submit<AddPointsCommand>(map);
}

void evaluate(Application& app)
{
  for (Tag* tag : app.scene.tags()) {
    auto* script_tag = type_cast<ScriptTag*>(tag);
    if (script_tag != nullptr) { script_tag->force_evaluate(); }
  }
}

void select_all(Application& app)
{
  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    for (auto* point : path->points()) {
      point->is_selected = true;
    }
  }
}
void deselect_all(Application& app)
{
  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    for (auto* point : path->points()) {
      point->is_selected = false;
    }
  }
}

void invert_selection(Application& app)
{
  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    for (auto* point : path->points()) {
      point->is_selected = !point->is_selected;
    }
  }
}

void convert_objects(Application& app)
{
  const auto convertables = ::filter_if(app.scene.item_selection<Object>(), [](const Object* o) {
    return !!(o->flags() & Object::Flag::Convertable);
  });
  app.scene.undo_stack.beginMacro(QObject::tr("convert"));
  for (auto&& c : convertables) {
    auto converted = c->convert();
    assert(!c->is_root());
    TreeOwningContext<Object> context(*converted, c->parent(), c);
    const auto properties = ::transform<Property*>(app.scene.find_reference_holders(*c));
    if (properties.size() > 0) {
      app.scene.submit<PropertiesCommand<ReferenceProperty>>(properties, converted.get());
    }
    context.subject.capture(std::move(converted));
    using object_tree_type = Tree<Object>;
    app.scene.submit<AddCommand<object_tree_type>>(app.scene.object_tree, std::move(context));
  }
  const auto selection = ::transform<Object*, std::set>(convertables, ::identity);
  using remove_command = RemoveCommand<Tree<Object>>;
  app.scene.template submit<remove_command>(app.scene.object_tree, selection);
  app.scene.undo_stack.endMacro();
}

}  // namespace omm::actions
