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
  const auto category = QObject::tr("tool").toStdString();
  create_property<OptionsProperty>(TANGENT_MODE_PROPERTY_KEY, 0)
    .set_options({ QObject::tr("Mirror").toStdString(), QObject::tr("Individual").toStdString() })
    .set_label(QObject::tr("tangent").toStdString())
    .set_category(category);

  create_property<OptionsProperty>(BOUNDING_BOX_MODE_PROPERTY_KEY, 0)
    .set_options( { QObject::tr("Include Tangents").toStdString(),
                    QObject::tr("Exclude Tangents").toStdString(),
                    QObject::tr("None").toStdString() })
    .set_label(QObject::tr("Bounding Box").toStdString())
    .set_category(category);
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

  for (Path* path : m_paths) {
    path->update();
  }

  scene.submit(std::make_unique<PointsTransformationCommand>(map));
}


bool SelectPointsBaseTool::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force)
{
  const auto paths = type_cast<Path*>(scene.template item_selection<Object>());
  m_initial_points.clear();
  m_paths.clear();
  Q_UNUSED(force);
  if (AbstractSelectTool::mouse_press(pos, event, false)
    || AbstractSelectTool::mouse_press(pos, event, true)) {
    for (Path* path : paths) {
      const std::vector<std::size_t> selected_points = path->selected_points();
      if (selected_points.size() > 0) {
        m_paths.insert(path);
      }
      for (const std::size_t i : selected_points) {
        m_initial_points.insert(std::make_pair(std::make_pair(path, i), path->point(i)));
      }
    }
    return true;
  } else {
    for (auto* path : paths) {
      for (auto* point : path->points_ref()) {
        point->is_selected = false;
      }
    }
    return false;
  }
}

bool SelectPointsBaseTool::has_transformation() const
{
  for (auto* path : type_cast<Path*>(scene.template item_selection<Object>())) {
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
  static const auto remove_tangents = [](const Point& point) { return point.nibbed(); };
  switch (property(BOUNDING_BOX_MODE_PROPERTY_KEY)->value<BoundingBoxMode>()) {
  case BoundingBoxMode::IncludeTangents:
    return BoundingBox(::transform<Point, std::vector>(scene.point_selection.points()));
  case BoundingBoxMode::ExcludeTangents:
    return BoundingBox(::transform<Point, std::vector>(scene.point_selection.points(),
                                                       remove_tangents));
  case BoundingBoxMode::None:
    [[ fallthrough ]];
  default:
    return BoundingBox();
  }
}

bool SelectPointsBaseTool::modifies_points() const { return true; }

void SelectPointsBaseTool::on_property_value_changed(Property *property)
{
  if (property == this->property(BOUNDING_BOX_MODE_PROPERTY_KEY)) {
    Q_EMIT scene.message_box.appearance_changed(*this);
  }
  AbstractSelectTool::on_property_value_changed(property);
}

Vec2f SelectPointsBaseTool::selection_center() const
{
  return scene.point_selection.center();
}

std::string SelectPointsTool::type() const { return TYPE; }

void SelectPointsTool::reset()
{
  handles.clear();
  make_handles(*this, false);
  handles.push_back(std::make_unique<BoundingBoxHandle<SelectPointsTool>>(*this));
}


}  // namespace omm
