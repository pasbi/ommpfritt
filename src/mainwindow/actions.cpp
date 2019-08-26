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
#include "geometry/cubics.h"
#include "commands/movecommand.h"

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
    return path->property(omm::Path::INTERPOLATION_PROPERTY_KEY);
  });

  if (map.size() > 0) {
    auto macro = app.scene.history.start_macro(QObject::tr("modify tangents"));
    using OptionsPropertyCommand = omm::PropertiesCommand<omm::OptionsProperty>;
    app.scene.submit<OptionsPropertyCommand>(interpolation_properties, bezier_mode);
    app.scene.submit<omm::ModifyPointsCommand>(map);
  }
}

void convert_objects(omm::Application& app, std::set<omm::Object*> convertables)
{
  using namespace omm;

  // spit convertables into [`convertables`, `leftover_convertables`]
  // s.t. `convertables` only contains top-level items, i.e. no item in `convertables` has a parent
  // in `convertables`. That's important because the children of a converted object must not change.
  // Porcess the left-over items later.
  std::set<Object*> leftover_convertables;
  {
    const auto all_convertables = convertables;
    Object::remove_internal_children(convertables);
    std::set_difference(all_convertables.begin(), all_convertables.end(),
        convertables.begin(), convertables.end(),
        std::inserter(leftover_convertables, leftover_convertables.end()));
  }

  if (convertables.size() > 0) {
    std::set<Object*> converted_objects;
    std::list<ObjectTreeMoveContext> move_contextes;
    for (auto&& c : convertables) {
      auto converted = c->convert();
      assert(!c->is_root());
      ObjectTreeOwningContext context(*converted, c->tree_parent(), c);
      const auto properties = ::transform<Property*>(app.scene.find_reference_holders(*c));
      if (properties.size() > 0) {
        app.scene.submit<PropertiesCommand<ReferenceProperty>>(properties, converted.get());
      }
      auto& converted_ref = *converted;
      context.subject.capture(std::move(converted));
      app.scene.submit<AddCommand<ObjectTree>>(app.scene.object_tree, std::move(context));
      converted_ref.set_transformation(c->transformation());
      converted_objects.insert(&converted_ref);

      const auto make_move_context = [&converted_ref](auto* cc) {
        return ObjectTreeMoveContext(*cc, converted_ref, nullptr);
      };
      const auto old_children = c->tree_children();
      std::transform(old_children.rbegin(), old_children.rend(),
                     std::back_inserter(move_contextes), make_move_context);
    }

    app.scene.template submit<MoveCommand<ObjectTree>>(app.scene.object_tree,
                                                       std::vector(move_contextes.begin(),
                                                                   move_contextes.end()));
    const auto selection = ::transform<Object*, std::set>(convertables, ::identity);
    using remove_command = RemoveCommand<ObjectTree>;
    app.scene.template submit<remove_command>(app.scene.object_tree, selection);
    app.scene.set_selection(down_cast(converted_objects));

    // process the left over items
    convert_objects(app, leftover_convertables);
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
    auto selected_points = path->selected_points();
    std::sort(selected_points.begin(), selected_points.end());
    for (std::size_t i = 0; i < selected_points.size(); ++i) { selected_points[i] -= i; }
    map[path] = selected_points;
  }
  if (!map.empty()) {
    app.scene.submit<RemovePointsCommand>(map);
    app.scene.update_tool();
  }
}

void subdivide(Application& app)
{
  constexpr auto n = 1;
  std::map<Path*, std::vector<Path::PointSequence>> map;

  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    std::list<Path::PointSequence> sequences;
    const auto cubics = path->cubics();
    for (std::size_t i = 0; i < cubics.n_segments(); ++i) {
      if (cubics.segment(i).is_selected()) {
        Path::PointSequence sequence(i+1);
        for (std::size_t j = 0; j < n; ++j) {
          sequence.sequence.push_back(cubics.segment(i).evaluate((j+1.0)/(n+1.0)));
        }
        sequences.push_back(sequence);
      } else {
      }
    }
    map[path] = std::vector(sequences.begin(), sequences.end());
  }

  if (map.size() > 0) {
    app.scene.submit<AddPointsCommand>(map);
    app.scene.tool_box.active_tool().reset();
  }
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
    for (auto* point : path->points_ref()) {
      point->is_selected = true;
    }
  }
  Q_EMIT app.scene.repaint();
}

void deselect_all(Application& app)
{
  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    for (auto* point : path->points_ref()) {
      point->is_selected = false;
    }
  }
  Q_EMIT app.scene.repaint();
}

void invert_selection(Application& app)
{
  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    for (auto* point : path->points_ref()) {
      point->is_selected = !point->is_selected;
    }
  }
  Q_EMIT app.scene.repaint();
}

void convert_objects(Application& app)
{
  const auto convertables = ::filter_if(app.scene.item_selection<Object>(), [](const Object* o) {
    return !!(o->flags() & Object::Flag::Convertable);
  });
  if (convertables.size() > 0) {
    Scene& scene = app.scene;
    auto macro = scene.history.start_remember_selection_macro(QObject::tr("convert"), scene);
    ::convert_objects(app, convertables);
  }
}

}  // namespace omm::actions
