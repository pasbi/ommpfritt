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
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/mailbox.h"
#include "scene/pointselection.h"
#include "scene/scene.h"
#include "scene/toplevelsplit.h"
#include "tools/toolbox.h"
#include "removeif.h"
#include <QUndoStack>
#include <functional>
#include <map>
#include <set>

namespace
{

using namespace omm;

template<typename F> void foreach_subpath(Application& app, F&& f)
{
  for (auto* path_object : app.scene->item_selection<PathObject>()) {
    for (auto* path : path_object->geometry().paths()) {
      f(path);
    }
  }
}

void modify_tangents(InterpolationMode mode, Application& app)
{
  std::map<PathPoint*, Point> map;
  const auto paths = app.scene->item_selection<PathObject>();
  for (PathObject* path_object : paths) {
    for (const Path* path : path_object->geometry().paths()) {
      const auto points = path->points();
      for (std::size_t i = 0; i < points.size(); ++i) {
        PathPoint* point = points[i];
        if (point->is_selected()) {
          switch (mode) {
          case InterpolationMode::Bezier:
            break;  // do nothing.
          case InterpolationMode::Smooth:
            map[point] = path->smoothen_point(i);
            break;
          case InterpolationMode::Linear:
            map[point] = point->geometry().nibbed();
          }
        }
      }
    }
  }

  constexpr auto bezier_mode = static_cast<std::size_t>(InterpolationMode::Bezier);
  const auto interpolation_properties = util::transform(paths, [](PathObject* path) {
    return path->property(PathObject::INTERPOLATION_PROPERTY_KEY);
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

void convert_object(Application& app,
         const Object& object_to_convert,
         std::deque<ObjectTreeMoveContext>& contextes,
         std::set<Object*>& converted_objects)
{
  bool keep_children = true;
  auto converted_object = object_to_convert.convert(keep_children);
  auto& ref = *converted_object;
  ref.set_object_tree(app.scene->object_tree());
  assert(!object_to_convert.is_root());
  ObjectTreeOwningContext context(ref, object_to_convert.tree_parent(), &object_to_convert);
  const auto properties = util::transform<Property*>(app.scene->find_reference_holders(object_to_convert));
  if (!properties.empty()) {
    app.scene->submit<PropertiesCommand<ReferenceProperty>>(properties, &ref);
  }
  context.subject.capture(std::move(converted_object));
  app.scene->submit<AddCommand<ObjectTree>>(app.scene->object_tree(), std::move(context));
  if (auto* const po = type_cast<PathObject*>(&ref); po != nullptr) {
    app.scene->submit<ShareJoinedPointsCommand>(*app.scene, po->geometry());
  }
  assert(ref.scene() == app.scene.get());
  ref.set_transformation(object_to_convert.transformation());
  converted_objects.insert(&ref);

  if (keep_children) {
    const auto old_children = object_to_convert.tree_children();
    std::transform(old_children.rbegin(),
                   old_children.rend(),
                   std::back_inserter(contextes),
                   [&ref](auto* cc) { return ObjectTreeMoveContext(*cc, ref, nullptr); });
  }
}

std::set<Object*> convert_objects_recursively(Application& app, const std::set<Object*>& convertibles)
{
  const TopLevelSplit split{convertibles};
  std::set<Object*> converted_objects;
  if (!split.top_level_objects().empty()) {
    std::deque<ObjectTreeMoveContext> move_contextes;
    for (const auto* object_to_convert : split.top_level_objects()) {
      convert_object(app, *object_to_convert, move_contextes, converted_objects);
    }

    app.scene->submit<MoveCommand<ObjectTree>>(app.scene->object_tree(), move_contextes);
    app.scene->submit<RemoveCommand<ObjectTree>>(app.scene->object_tree(), split.top_level_objects());

    // process the left over items
    const auto cos = convert_objects_recursively(app, split.non_top_level_objects());
    converted_objects.insert(cos.begin(), cos.end());
  }
  return converted_objects;
}

void remove_selected_points(Application& app)
{
  std::unique_ptr<Macro> macro;
  for (auto* path_object : app.scene->item_selection<PathObject>()) {
    std::deque<PathView> removed_points;
    for (Path* path : path_object->geometry().paths()) {
      const auto selected_ranges = find_coherent_ranges(path->points(),
                                                        std::mem_fn(&PathPoint::is_selected));
      for (const auto& range : selected_ranges) {
        removed_points.emplace_back(*path, range.start, range.size);
      }
    }

    if (!removed_points.empty()) {
      auto command = std::make_unique<RemovePointsCommand>(*path_object, std::move(removed_points));
      if (!macro) {
        macro = app.scene->history().start_macro(command->actionText());
      }
      app.scene->submit(std::move(command));
      app.scene->update_tool();
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
  for (auto* path : app.scene->item_selection<PathObject>()) {
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
    for (auto* path_object : app.scene->item_selection<PathObject>()) {
      for (auto* point : path_object->geometry().points()) {
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
    for (auto* path_object : app.scene->item_selection<PathObject>()) {
      for (auto* point : path_object->geometry().points()) {
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
    for (auto* path_object : app.scene->item_selection<PathObject>()) {
      for (auto* point : path_object->geometry().points()) {
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
  std::set<const Path*> selected_paths;
  foreach_subpath(app, [&selected_paths](const auto* path) {
    const auto points = path->points();
    if (std::any_of(points.begin(), points.end(), std::mem_fn(&PathPoint::is_selected))) {
      selected_paths.insert(path);
    }
  });

  for (bool selected_paths_changed = true; selected_paths_changed;) {
    selected_paths_changed = false;
    for (const auto* path : selected_paths) {
      for (auto* point : path->points()) {
        for (auto* joined_point : point->joined_points()) {
          const auto& other_path = joined_point->path();
          if (const auto [_, was_inserted] = selected_paths.insert(&other_path); was_inserted) {
            selected_paths_changed = true;
          }
        }
      }
    }
  }

  for (const auto* path : selected_paths) {
    for (auto* point : path->points()) {
      point->set_selected(true);
    }
  }

  Q_EMIT app.mail_box().scene_appearance_changed();
}

void fill_selection(Application& app)
{
  foreach_subpath(app, [](const auto* segment) {
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
  foreach_subpath(app, [](const auto* segment) {
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
  foreach_subpath(app, [](const auto* segment) {
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
  {"convert objects", path_actions::convert_objects},
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

std::set<Object*> convert_objects(Application& app)
{
  const auto convertibles = util::remove_if(app.scene->item_selection<Object>(), [](const Object* o) {
    return !(o->flags() & Flag::Convertible);
  });
  if (!convertibles.empty()) {
    Scene& scene = *app.scene;
    auto macro = scene.history().start_macro(QObject::tr("convert"));
    scene.submit<ObjectSelectionCommand>(*app.scene, convertibles);
    auto converted_objects = convert_objects_recursively(app, convertibles);
    scene.submit<ObjectSelectionCommand>(*app.scene, converted_objects);
    const auto is_path = [](auto&& object) { return object->type() == PathObject::TYPE; };
    if (std::all_of(converted_objects.begin(), converted_objects.end(), is_path)) {
      scene.set_mode(SceneMode::Vertex);
    }
    return converted_objects;
  }
  return {};
}

bool perform_action(Application& app, const QString& name)
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
