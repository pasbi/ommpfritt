#include "tools/selectpointstool.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "objects/path.h"
#include "properties/optionsproperty.h"
#include "tools/handles/boundingboxhandle.h"

namespace omm
{

SelectPointsBaseTool::SelectPointsBaseTool(Scene& scene) : AbstractSelectTool(scene)
{
  add_property<OptionsProperty>(TANGENT_MODE_PROPERTY_KEY, 0)
    .set_options({ QObject::tr("Mirror").toStdString(), QObject::tr("Individual").toStdString() })
    .set_label(QObject::tr("tangent").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

PointSelectHandle::TangentMode SelectPointsBaseTool::tangent_mode() const
{
  const auto i = property(TANGENT_MODE_PROPERTY_KEY)->value<size_t>();
  return static_cast<PointSelectHandle::TangentMode>(i);
}

std::unique_ptr<QMenu> SelectPointsBaseTool::make_context_menu(QWidget* parent)
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

void SelectPointsBaseTool::on_selection_changed() { on_scene_changed(); }

void SelectPointsBaseTool::transform_objects(ObjectTransformation t)
{
  class TransformationCache : public Cache<Path*, ObjectTransformation>
  {
  public:
    TransformationCache(const Matrix& mat) : m_mat(mat) {}
    ObjectTransformation retrieve(Path* const& path) const {
      const Matrix gt = path->global_transformation(false).to_mat();
      return ObjectTransformation(gt.inverted() * m_mat * gt);
    }
  private:
    const Matrix m_mat;
  };

  TransformationCache cache(t.to_mat());

  PointsTransformationCommand::Map map;
  for (auto&& [key, point] : m_initial_points) {
    const ObjectTransformation premul = cache.get(key.first);
    auto p = premul.apply(point);
    p.is_selected = point.is_selected;
    map.insert(std::pair(key, p));
  }

  scene.submit(std::make_unique<PointsTransformationCommand>(map));
}

std::set<Path *> SelectPointsBaseTool::paths() const
{
  return type_cast<Path*>(scene.item_selection<Object>());
}

bool SelectPointsBaseTool::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force)
{
  m_initial_points.clear();
  Q_UNUSED(force);
  if (AbstractSelectTool::mouse_press(pos, event, false)
    || AbstractSelectTool::mouse_press(pos, event, true)) {
    for (Path* path : paths()) {
      for (const std::size_t i : path->selected_points()) {
        m_initial_points.insert(std::make_pair(std::make_pair(path, i), path->point(i)));
      }
    }
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

bool SelectPointsBaseTool::has_transformation() const
{
  for (auto* path : paths()) {
    for (auto* point : path->points_ref()) {
      if (point->is_selected) {
        return true;
      }
    }
  }
  return false;
}

BoundingBox SelectPointsBaseTool::bounding_box() const
{
  return BoundingBox(::transform<Point, std::vector>(selected_points()));
}

std::set<Point> SelectPointsBaseTool::selected_points() const
{
  std::set<Point> selected_points;
  for (auto* path : paths()) {
    for (auto* point : path->points_ref()) {
      if (point->is_selected) {
        selected_points.insert(path->global_transformation(false).apply(*point));
      }
    }
  }
  return selected_points;
}

Vec2f SelectPointsBaseTool::selection_center() const
{
  const auto selected_points = this->selected_points();
  Vec2f sum(0.0, 0.0);
  for (const Point& p : selected_points) {
    sum += p.position;
  }
  return sum / static_cast<double>(selected_points.size());
}

std::string SelectPointsTool::type() const { return TYPE; }

void SelectPointsTool::on_scene_changed()
{
  handles.clear();
  make_handles(*this, false);
  handles.push_back(std::make_unique<BoundingBoxHandle<SelectPointsTool>>(*this));
}


}  // namespace omm
