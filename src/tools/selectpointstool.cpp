#include "tools/selectpointstool.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "commands/pointstransformationcommand.h"
#include "objects/path.h"
#include "properties/optionsproperty.h"

namespace omm
{

SelectPointsTool::SelectPointsTool(Scene& scene) : AbstractSelectTool(scene)
{
  add_property<OptionsProperty>(TANGENT_MODE_PROPERTY_KEY, 0)
    .set_options({ QObject::tr("Mirror").toStdString(), QObject::tr("Individual").toStdString() })
    .set_label(QObject::tr("tangent").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

std::string SelectPointsTool::type() const { return TYPE; }

PointSelectHandle::TangentMode SelectPointsTool::tangent_mode() const
{
  const auto i = property(TANGENT_MODE_PROPERTY_KEY)->value<size_t>();
  return static_cast<PointSelectHandle::TangentMode>(i);
}

std::unique_ptr<QMenu> SelectPointsTool::make_context_menu(QWidget* parent)
{
  Q_UNUSED(parent)
  auto& app = Application::instance();
  auto menus = app.key_bindings.make_menus(app, MainWindow::path_menu_entries());
  if (menus.size() > 1) {
    LWARNING << "cannot combine entries from multiple menus";
    // TODO replace top-level-menu with custom key (i.e. 'path' in this case).
    // then, all entries will be in the same menu.
  }
  if (menus.size() == 0) { return nullptr; }
  return std::move(menus.front());
}

void SelectPointsTool::on_selection_changed() { on_scene_changed(); }

Command* SelectPointsTool::transform_objects(ObjectTransformation t, const bool tool_space)
{
  if (tool_space) { t = t.transformed(this->transformation().inverted()); }
  const auto paths = this->paths();
  if (paths.size() > 0) {
    auto command = std::make_unique<PointsTransformationCommand>(paths, t);
    auto& command_ref = *command;
    scene.submit(std::move(command));
    return &command_ref;
  } else {
    return nullptr;
  }
}

void SelectPointsTool::on_scene_changed()
{
  handles.clear();
  make_handles(*this, false);
}

std::set<Path *> SelectPointsTool::paths() const
{
  return type_cast<Path*>(scene.item_selection<Object>());
}

bool SelectPointsTool::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force)
{
  Q_UNUSED(force);
  if (AbstractSelectTool::mouse_press(pos, event, false)) {
    return true;
  } else if (AbstractSelectTool::mouse_press(pos, event, true)) {
    return true;
  } else {
    for (auto* path : paths()) {
      for (auto* point : path->points_ref()) {
        point->is_selected = false;
      }
    }
    return false;
  }
}

bool SelectPointsTool::has_transformation() const
{
  return !selected_points().empty();
}

std::set<Point *> SelectPointsTool::selected_points() const
{
  std::set<Point*> selected_points;
  for (auto* path : paths()) {
    for (auto* point : path->points_ref()) {
      if (point->is_selected) {
        selected_points.insert(point);
      }
    }
  }
  return selected_points;
}

Vec2f SelectPointsTool::selection_center() const
{
  std::set<Vec2f> ps;
  for (auto* path : paths()) {
    for (auto* point : path->points_ref()) {
      if (point->is_selected) {
        ps.insert(path->global_transformation().apply_to_position(point->position));
      }
    }
  }
  return std::accumulate(ps.begin(), ps.end(), Vec2f(0.0, 0.0)) / static_cast<double>(ps.size());
}

}  // namespace omm
