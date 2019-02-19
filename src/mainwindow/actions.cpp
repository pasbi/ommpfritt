#include "mainwindow/actions.h"
#include "scene/scene.h"
#include "commands/modifypointscommand.h"
#include "commands/propertycommand.h"
#include "commands/addcommand.h"
#include "commands/removecommand.h"
#include "properties/optionsproperty.h"
#include "tags/scripttag.h"
#include "mainwindow/application.h"
#include "keybindings/keybindingsdialog.h"
#include "mainwindow/mainwindow.h"
#include <QUndoStack>
#include "common.h"
#include "properties/referenceproperty.h"

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

void show_keybindings_dialog()
{
  auto& app = Application::instance();
  KeyBindingsDialog(app.key_bindings, app.main_window()).exec();
}

void previous_tool() { scene().tool_box.set_previous_tool(); }

void select_all()
{
  for (auto* path : Object::cast<Path>(scene().object_selection())) {
    for (auto* point : path->points()) {
      point->is_selected = true;
    }
  }
}
void deselect_all()
{
  for (auto* path : Object::cast<Path>(scene().object_selection())) {
    for (auto* point : path->points()) {
      point->is_selected = false;
    }
  }
}

void invert_selection()
{
  for (auto* path : Object::cast<Path>(scene().object_selection())) {
    for (auto* point : path->points()) {
      point->is_selected = !point->is_selected;
    }
  }
}

void convert_objects()
{
  const auto convertables = ::filter_if(scene().object_selection(), [](const Object* object) {
    return !!(object->flags() & Object::Flag::Convertable);
  });
  scene().undo_stack.beginMacro(QObject::tr("convert"));
  for (auto&& c : convertables) {
    auto converted = c->convert();
    assert(!c->is_root());
    TreeOwningContext<Object> context(*converted, c->parent(), c);
    const auto properties = ::transform<Property*>(scene().find_reference_holders(*c));
    if (properties.size() > 0) {
      scene().submit<PropertiesCommand<ReferenceProperty>>(properties, converted.get());
    }
    context.subject.capture(std::move(converted));
    using object_tree_type = Tree<Object>;
    scene().submit<AddCommand<object_tree_type>>(scene().object_tree, std::move(context));
  }
  const auto selection = ::transform<Object*, std::set>(convertables, ::identity);
  using remove_command = RemoveCommand<Tree<Object>>;
  scene().template submit<remove_command>(scene().object_tree, selection);
  scene().undo_stack.endMacro();
}

void remove_selection()
{
  scene().remove(omm::Application::instance().main_window(), scene().selection());
}

void new_style()
{
  using command_type = AddCommand<List<Style>>;
  scene().submit<command_type>(scene().styles, scene().default_style().clone());
}

}  // namespace omm::actions
