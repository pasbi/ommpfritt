#include "mainwindow/pathactions.h"
#include "commands/addcommand.h"
#include "commands/joinpointscommand.h"
#include "commands/modifypointscommand.h"
#include "commands/movecommand.h"
#include "commands/objectselectioncommand.h"
#include "commands/propertycommand.h"
#include "commands/removecommand.h"
#include "commands/subdividepathcommand.h"
#include "common.h"
#include "main/application.h"
#include "mainwindow/mainwindow.h"
#include "properties/optionproperty.h"
#include "properties/referenceproperty.h"
#include "objects/path.h"
#include "objects/pathpoint.h"
#include "objects/segment.h"
#include "objects/convertedobject.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/mailbox.h"
#include "scene/pointselection.h"
#include "scene/scene.h"
#include "tools/toolbox.h"
#include <QUndoStack>
#include <functional>
#include <map>
#include <set>

namespace
{

using namespace omm;

template<typename F> void foreach_segment(Application& app, F&& f)
{
  for (auto* path : app.scene->item_selection<Path>()) {
    for (auto* segment : path->segments()) {
      f(segment);
    }
  }
}

void modify_tangents(InterpolationMode mode, Application& app)
{
  std::map<PathPoint*, Point> map;
  const auto paths = app.scene->item_selection<Path>();
  for (Path* path : paths) {
    for (const Segment* segment : path->segments()) {
      const auto points = segment->points();
      for (std::size_t i = 0; i < points.size(); ++i) {
        PathPoint* point = points[i];
        if (point->is_selected()) {
          switch (mode) {
          case InterpolationMode::Bezier:
            break;  // do nothing.
          case InterpolationMode::Smooth:
            map[point] = segment->smoothen_point(i);
            break;
          case InterpolationMode::Linear:
            map[point] = point->geometry().nibbed();
          }
        }
      }
    }
  }

  constexpr auto bezier_mode = static_cast<std::size_t>(InterpolationMode::Bezier);
  const auto interpolation_properties = ::transform<Property*>(paths, [](Path* path) {
    return path->property(Path::INTERPOLATION_PROPERTY_KEY);
  });

  if (!map.empty()) {
    auto macro = app.scene->history().start_macro(QObject::tr("modify tangents"));
    using OptionPropertyCommand = PropertiesCommand<OptionProperty>;
    app.scene->submit<OptionPropertyCommand>(interpolation_properties, bezier_mode);
    app.scene->submit<ModifyPointsCommand>(map);
  }
}

void modify_tangents_linear(Application& app)
{
  modify_tangents(InterpolationMode::Linear, app);
}

void modify_tangents_smooth(Application& app)
{
  modify_tangents(InterpolationMode::Smooth, app);
}

std::set<Object*> convert_objects_recursively(Application& app, std::set<Object*> convertibles)
{
  // split convertibles into [`convertibles`, `leftover_convertibles`]
  // s.t. `convertibles` only contains top-level items, i.e. no item in `convertibles` has a parent
  // in `convertibles`. That's important because the children of a converted object must not change.
  // Porcess the left-over items later.
  std::set<Object*> leftover_convertibles;
  {
    const auto all_convertibles = convertibles;
    Object::remove_internal_children(convertibles);
    std::set_difference(all_convertibles.begin(),
                        all_convertibles.end(),
                        convertibles.begin(),
                        convertibles.end(),
                        std::inserter(leftover_convertibles, leftover_convertibles.end()));
  }

  std::set<Object*> converted_objects;
  if (!convertibles.empty()) {
    std::list<ObjectTreeMoveContext> move_contextes;
    for (auto&& c : convertibles) {
      auto [converted, joined_points, move_children] = c->convert();

      converted->set_object_tree(app.scene->object_tree());
      assert(!c->is_root());
      ObjectTreeOwningContext context(*converted, c->tree_parent(), c);
      const auto properties = ::transform<Property*>(app.scene->find_reference_holders(*c));
      if (!properties.empty()) {
        app.scene->submit<PropertiesCommand<ReferenceProperty>>(properties, converted.get());
      }
      auto& converted_ref = *converted;
      context.subject.capture(std::move(converted));
      app.scene->submit<AddCommand<ObjectTree>>(app.scene->object_tree(), std::move(context));
      app.scene->submit<JoinPointsCommand>(*app.scene, std::move(*joined_points));
      converted_ref.set_transformation(c->transformation());
      converted_objects.insert(&converted_ref);

      if (move_children) {
        const auto make_move_context = [&converted_ref](auto* cc) {
          return ObjectTreeMoveContext(*cc, converted_ref, nullptr);
        };
        const auto old_children = c->tree_children();
        std::transform(old_children.rbegin(),
                       old_children.rend(),
                       std::back_inserter(move_contextes),
                       make_move_context);
      }
    }

    app.scene->template submit<MoveCommand<ObjectTree>>(
        app.scene->object_tree(),
        std::vector(move_contextes.begin(), move_contextes.end()));
    const auto selection = ::transform<Object*, std::set>(convertibles, ::identity);
    using remove_command = RemoveCommand<ObjectTree>;
    app.scene->template submit<remove_command>(app.scene->object_tree(), selection);

    // process the left over items
    const auto cos = convert_objects_recursively(app, leftover_convertibles);
    converted_objects.insert(cos.begin(), cos.end());
  }
  return converted_objects;
}

void remove_selected_points(Application& app)
{
  std::unique_ptr<Macro> macro;
  for (auto* path : app.scene->item_selection<Path>()) {
    std::deque<SegmentView> removed_points;
    for (Segment* segment : path->segments()) {
      const auto selected_ranges = find_coherent_ranges(segment->points(),
                                                        std::mem_fn(&PathPoint::is_selected));
      for (const auto& range : selected_ranges) {
        removed_points.emplace_back(*segment, range.start, range.size);
      }
    }

    if (!removed_points.empty()) {
      auto command = std::make_unique<RemovePointsCommand>(*path, std::move(removed_points));
      if (!macro) {
        macro = app.scene->history().start_macro(command->actionText());
      }
      app.scene->submit(std::move(command));
      app.scene->update_tool();
    }
  }
}

void convert_objects(Application& app)
{
  const auto convertibles
      = ::filter_if(app.scene->item_selection<Object>(),
                    [](const Object* o) { return !!(o->flags() & Flag::Convertible); });
  if (!convertibles.empty()) {
    Scene& scene = *app.scene;
    auto macro = scene.history().start_macro(QObject::tr("convert"));
    scene.submit<ObjectSelectionCommand>(*app.scene, convertibles);
    const auto converted_objects = convert_objects_recursively(app, convertibles);
    scene.submit<ObjectSelectionCommand>(*app.scene, converted_objects);
    const auto is_path = [](auto&& object) { return object->type() == Path::TYPE; };
    if (std::all_of(converted_objects.begin(), converted_objects.end(), is_path)) {
      scene.set_mode(SceneMode::Vertex);
    }
  }
}

void remove_selected_items(Application& app)
{
  switch (app.scene_mode()) {
  case SceneMode::Vertex:
    remove_selected_points(app);
    break;
  case SceneMode::Object:
    app.scene->remove(app.main_window(), app.scene->selection());
    break;
  }
}

void subdivide(Application& app)
{
  std::list<std::unique_ptr<SubdividePathCommand>> cmds;
  for (auto* path : app.scene->item_selection<Path>()) {
    auto cmd = std::make_unique<SubdividePathCommand>(*path);
    if (!cmd->is_noop()) {
      cmds.push_back(std::move(cmd));
    }
  }

  std::unique_ptr<Macro> macro;
  if (!cmds.empty()) {
    macro = app.scene->history().start_macro(QObject::tr("Subdivide Paths"));
  }
  for (auto&& cmd : cmds) {
    app.scene->submit(std::move(cmd));
  }
}

void select_all(Application& app)
{
  switch (app.scene_mode()) {
  case SceneMode::Vertex:
    for (auto* path : app.scene->item_selection<Path>()) {
      for (auto* point : path->points()) {
        point->set_selected(true);
      }
    }
    Q_EMIT app.scene->mail_box().point_selection_changed();
    break;
  case SceneMode::Object:
    app.scene->set_selection(down_cast(app.scene->object_tree().items()));
    break;
  }
  Q_EMIT app.mail_box().scene_appearance_changed();
}

void deselect_all(Application& app)
{
  switch (app.scene_mode()) {
  case SceneMode::Vertex:
    for (auto* path : app.scene->item_selection<Path>()) {
      for (auto* point : path->points()) {
        point->set_selected(false);
      }
    }
    Q_EMIT app.scene->mail_box().point_selection_changed();
    break;
  case SceneMode::Object:
    app.scene->set_selection({});
    break;
  }
  Q_EMIT app.mail_box().scene_appearance_changed();
}

template<typename T>
std::set<T> set_difference(const std::set<T>& set1, const std::set<T>& set2)
{
  std::set<T> difference;
  std::set_difference(set1.begin(), set1.end(), set2.begin(), set2.end(),
                      std::inserter(difference, difference.begin()));
  return difference;
}

void invert_selection(Application& app)
{
  switch (app.scene_mode()) {
  case SceneMode::Vertex:
    for (auto* path : app.scene->item_selection<Path>()) {
      for (auto* point : path->points()) {
        point->set_selected(!point->is_selected());
      }
    }
    Q_EMIT app.scene->mail_box().point_selection_changed();
    break;
  case SceneMode::Object:
    app.scene->set_selection(down_cast(set_difference(app.scene->object_tree().items(),
                                                      app.scene->item_selection<Object>())));
    break;
  }
  Q_EMIT app.mail_box().scene_appearance_changed();
}

void select_connected_points(Application& app)
{
  foreach_segment(app, [](const auto* segment) {
    const auto points = segment->points();
    if (std::any_of(points.begin(), points.end(), std::mem_fn(&PathPoint::is_selected))) {
      std::for_each(points.begin(), points.end(), [](auto* point) { point->set_selected(true); });
    }
  });
  Q_EMIT app.mail_box().scene_appearance_changed();
}

void fill_selection(Application& app)
{
  foreach_segment(app, [](const auto* segment) {
    const auto points = segment->points();
    auto first_it = std::find_if(points.begin(), points.end(), std::mem_fn(&PathPoint::is_selected));
    auto last_it = std::find_if(points.rbegin(), points.rend(), std::mem_fn(&PathPoint::is_selected));
    if (first_it != points.end() && last_it != points.rend()) {
      std::for_each(first_it, last_it.base(), [](auto* point) { point->set_selected(true); });
    }
  });
  Q_EMIT app.mail_box().scene_appearance_changed();
}

void extend_selection(Application& app)
{
  foreach_segment(app, [](const auto* segment) {
    std::set<std::size_t> selection;
    const auto points = segment->points();
    for (std::size_t i = 1; i < points.size() - 1; ++i) {
      if (points[i]->is_selected()) {
        selection.insert(i - 1);
        selection.insert(i + 1);
      }
    }
    for (const auto& i : selection) {
      points[i]->set_selected(true);
    }
  });
  Q_EMIT app.mail_box().scene_appearance_changed();
}

void shrink_selection(Application& app)
{
  foreach_segment(app, [](const auto* segment) {
    std::set<std::size_t> selection_fringe;
    const auto points = segment->points();
    for (std::size_t i = 1; i < points.size() - 1; ++i) {
      if (!points[i - 1]->is_selected() || !points[i + 1]->is_selected()) {
        selection_fringe.insert(i);
      }
    }
    for (auto&& i : selection_fringe) {
      points[i]->set_selected(false);
    }
  });
  Q_EMIT app.mail_box().scene_appearance_changed();
}

void join_points(Application& app)
{
  app.scene->submit<JoinPointsCommand>(*app.scene, std::deque{app.scene->point_selection->points()});
}

void disjoin_points(Application& app)
{
  app.scene->submit<DisjoinPointsCommand>(*app.scene, std::deque{app.scene->point_selection->points()});
}

const std::map<QString, std::function<void(Application& app)>> actions{
  {"make linear", modify_tangents_linear},
  {"make smooth", modify_tangents_smooth},
  {"remove selected items", remove_selected_items},
  {"subdivide", subdivide},
  {"select all", select_all},
  {"deselect all", deselect_all},
  {"invert selection", invert_selection},
  {"convert objects", convert_objects},
  {"select connected points", select_connected_points},
  {"fill selection", fill_selection},
  {"extend selection", extend_selection},
  {"shrink selection", shrink_selection},
  {"disjoin points", disjoin_points},
  {"join points", join_points},
};

}  // namespace

namespace omm::path_actions
{
bool perform_action(const QString& name, Application& app)
{
  auto it = ::actions.find(name);
  if (it == ::actions.end()) {
    return false;
  } else {
    it->second(app);
    return true;
  }
}

std::set<QString> available_actions()
{
  std::set<QString> names;
  const auto get_first = [](auto&& pair) { return pair.first; };
  std::transform(::actions.begin(), ::actions.end(), std::inserter(names, names.end()), get_first);
  return names;
}

}  // namespace omm::path_actions
