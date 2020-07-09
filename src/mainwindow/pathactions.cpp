#include "mainwindow/pathactions.h"
#include <set>
#include "commands/subdividepathcommand.h"
#include "scene/scene.h"
#include "commands/modifypointscommand.h"
#include "commands/propertycommand.h"
#include "commands/addcommand.h"
#include "commands/removecommand.h"
#include "properties/optionproperty.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include <QUndoStack>
#include "common.h"
#include "properties/referenceproperty.h"
#include "commands/movecommand.h"
#include "commands/objectselectioncommand.h"
#include "scene/history/historymodel.h"
#include "scene/messagebox.h"
#include "tools/toolbox.h"
#include <map>
#include <functional>

namespace
{

using namespace omm;

void modify_tangents(omm::InterpolationMode mode, omm::Application& app)
{
  using namespace omm;
  const auto paths = Object::cast<Path>(app.scene.item_selection<Object>());
  std::map<Path::iterator, omm::Point> map;
  for (omm::Path* path : paths) {
    const bool is_closed = path->is_closed();
    for (std::size_t s = 0; s < path->segments.size(); ++s) {
      const Path::Segment& segment = path->segments[s];
      const std::size_t n = segment.size();
      for (std::size_t i = 0; i < n; ++i) {
        if (segment[i].is_selected) {
          const Path::iterator it{*path, s, i};
          switch (mode) {
          case InterpolationMode::Bezier:
            break;  // do nothing.
          case InterpolationMode::Smooth:
            map[it] = Path::smoothen_point(segment, is_closed, i);
            break;
          case InterpolationMode::Linear:
            map[it] = segment[i].nibbed();
          }
        }
      }
    }
  }

  constexpr auto bezier_mode = static_cast<std::size_t>(omm::InterpolationMode::Bezier);
  const auto interpolation_properties = ::transform<omm::Property*>(paths, [](omm::Path* path) {
    return path->property(omm::Path::INTERPOLATION_PROPERTY_KEY);
  });

  if (map.size() > 0) {
    auto macro = app.scene.history().start_macro(QObject::tr("modify tangents"));
    using OptionPropertyCommand = omm::PropertiesCommand<omm::OptionProperty>;
    app.scene.submit<OptionPropertyCommand>(interpolation_properties, bezier_mode);
    app.scene.submit<omm::ModifyPointsCommand>(map);
  }
}

std::set<omm::Object*> convert_objects(omm::Application& app, std::set<omm::Object*> convertibles)
{
  using namespace omm;

  // split convertibles into [`convertibles`, `leftover_convertibles`]
  // s.t. `convertibles` only contains top-level items, i.e. no item in `convertibles` has a parent
  // in `convertibles`. That's important because the children of a converted object must not change.
  // Porcess the left-over items later.
  std::set<Object*> leftover_convertibles;
  {
    const auto all_convertibles = convertibles;
    Object::remove_internal_children(convertibles);
    std::set_difference(all_convertibles.begin(), all_convertibles.end(),
        convertibles.begin(), convertibles.end(),
        std::inserter(leftover_convertibles, leftover_convertibles.end()));
  }

  std::set<Object*> converted_objects;
  if (convertibles.size() > 0) {
    std::list<ObjectTreeMoveContext> move_contextes;
    for (auto&& c : convertibles) {
      auto [converted, move_children] = c->convert();

      converted->set_object_tree(app.scene.object_tree());
      assert(!c->is_root());
      ObjectTreeOwningContext context(*converted, c->tree_parent(), c);
      const auto properties = ::transform<Property*>(app.scene.find_reference_holders(*c));
      if (properties.size() > 0) {
        app.scene.submit<PropertiesCommand<ReferenceProperty>>(properties, converted.get());
      }
      auto& converted_ref = *converted;
      context.subject.capture(std::move(converted));
      app.scene.submit<AddCommand<ObjectTree>>(app.scene.object_tree(), std::move(context));
      converted_ref.set_transformation(c->transformation());
      converted_objects.insert(&converted_ref);

      if (move_children) {
        const auto make_move_context = [&converted_ref](auto* cc) {
          return ObjectTreeMoveContext(*cc, converted_ref, nullptr);
        };
        const auto old_children = c->tree_children();
        std::transform(old_children.rbegin(), old_children.rend(),
                       std::back_inserter(move_contextes), make_move_context);
      }
    }

    app.scene.template submit<MoveCommand<ObjectTree>>(app.scene.object_tree(),
                                                       std::vector(move_contextes.begin(),
                                                                   move_contextes.end()));
    const auto selection = ::transform<Object*, std::set>(convertibles, ::identity);
    using remove_command = RemoveCommand<ObjectTree>;
    app.scene.template submit<remove_command>(app.scene.object_tree(), selection);

    // process the left over items
    const auto cos = convert_objects(app, leftover_convertibles);
    converted_objects.insert(cos.begin(), cos.end());
  }
  return converted_objects;
}

void remove_selected_points(Application& app)
{
  std::unique_ptr<Macro> macro;
  for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
    std::vector<RemovePointsCommand::Range> removed_points;
    auto last = path->end();
    const auto is_contiguos = [&last](const Path::iterator& it) {
      if (it.path != last.path || it.segment != last.segment) {
        return false;
      } else {
        return it.point == last.point + 1;
      }
    };
    for (auto it = path->begin(); it != path->end(); ++it) {
      if (it->is_selected) {
        if (is_contiguos(it)) {
          removed_points.back().length += 1;
        } else {
          removed_points.push_back(RemovePointsCommand::Range{it, 1});
        }
        last = it;
      }
    }
    if (!removed_points.empty()) {
      auto command = std::make_unique<RemovePointsCommand>(*path, removed_points);
      if (!macro) {
        macro = app.scene.history().start_macro(command->label());
      }
      app.scene.submit(std::move(command));
      app.scene.update_tool();
    }
  }
}

using namespace omm;

const std::map<QString, std::function<void(Application& app)>> actions {
  {"make linear", [](Application& app) { modify_tangents(InterpolationMode::Linear, app); }},
  {"make smooth", [](Application& app) { modify_tangents(InterpolationMode::Smooth, app); }},

  {"remove selected items", [](Application& app) {
    switch (app.scene_mode()) {
    case SceneMode::Vertex:
      remove_selected_points(app);
      break;
    case SceneMode::Object:
      app.scene.remove(app.main_window(), app.scene.selection());
      break;
    }
  }},

  {"subdivide", [](Application& app) {
    Q_UNUSED(app)
    std::list<std::unique_ptr<SubdividePathCommand>> cmds;
    for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
      if (path) {
        auto cmd = std::make_unique<SubdividePathCommand>(*path);
        if (!cmd->is_noop()) {
          cmds.push_back(std::move(cmd));
        }
      }
    }

    std::unique_ptr<Macro> macro;
    if (cmds.size() > 0) {
      macro = app.scene.history().start_macro(QObject::tr("Subdivide Paths"));
    }
    for (auto&& cmd : cmds) {
      app.scene.submit(std::move(cmd));
    }
  }},

  {"select all", [](Application& app) {
    switch (app.scene_mode()) {
    case SceneMode::Vertex:
      for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
        for (auto&& point : *path) {
          point.is_selected = true;
        }
      }
      break;
    case SceneMode::Object:
      app.scene.set_selection(down_cast(app.scene.object_tree().items()));
      break;
    }
    Q_EMIT app.message_box().appearance_changed();
  }},

  {"deselect all", [](Application& app) {
    switch (app.scene_mode()) {
    case SceneMode::Vertex:
      for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
        for (auto&& point : *path) {
          point.is_selected = false;
        }
      }
      break;
    case SceneMode::Object:
      app.scene.set_selection({});
      break;
    }
    Q_EMIT app.message_box().appearance_changed();
  }},

  {"invert selection", [](Application& app) {
    switch (app.scene_mode()) {
    case SceneMode::Vertex:
      for (auto* path : Object::cast<Path>(app.scene.item_selection<Object>())) {
        for (auto&& point : *path) {
          point.is_selected = !point.is_selected;
        }
      }
      break;
    case SceneMode::Object:
    {
      const auto object_selection = app.scene.item_selection<Object>();
      auto difference = app.scene.object_tree().items();
      for (auto&& s : object_selection) {
        difference.erase(s);
      }
      app.scene.set_selection(down_cast(difference));
      break;
    }
    }
    Q_EMIT app.message_box().appearance_changed();
  }},

  {"convert objects", [](Application& app) {
    const auto convertibles = ::filter_if(app.scene.item_selection<Object>(), [](const Object* o) {
      return !!(o->flags() & Flag::Convertible);
    });
    if (convertibles.size() > 0) {
      Scene& scene = app.scene;
      auto macro = scene.history().start_macro(QObject::tr("convert"));
      scene.submit<ObjectSelectionCommand>(app.scene, convertibles);
      const auto converted_objects = ::convert_objects(app, convertibles);
      scene.submit<ObjectSelectionCommand>(app.scene, converted_objects);
      const auto is_path = [](auto&& object) { return object->type() == Path::TYPE; };
      if (std::all_of(converted_objects.begin(), converted_objects.end(), is_path)) {
        scene.set_mode(SceneMode::Vertex);
      }
    }
  }},

  {"remove unused styles", [](Application& app) {
    auto& scene = app.scene;
    const auto unused_styles = ::filter_if(app.scene.styles().items(), [&scene](const auto* style) {
      return scene.find_reference_holders(*style).empty();
    });
    scene.submit<RemoveCommand<StyleList>>(scene.styles(), unused_styles);
  }},
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




}  // namespace omm::actions
