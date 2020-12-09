#include "tools/selectpointstool.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "objects/path.h"
#include "properties/optionproperty.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tools/handles/boundingboxhandle.h"

namespace omm
{
SelectPointsBaseTool::SelectPointsBaseTool(Scene& scene)
    : AbstractSelectTool(scene), m_transform_points_helper(scene, Space::Viewport)
{
  const auto category = QObject::tr("tool");
  create_property<OptionProperty>(TANGENT_MODE_PROPERTY_KEY, 0)
      .set_options({QObject::tr("Mirror"), QObject::tr("Individual")})
      .set_label(QObject::tr("tangent"))
      .set_category(category)
      .set_animatable(false);

  create_property<OptionProperty>(BOUNDING_BOX_MODE_PROPERTY_KEY, 1)
      .set_options(
          {QObject::tr("Include Tangents"), QObject::tr("Exclude Tangents"), QObject::tr("None")})
      .set_label(QObject::tr("Bounding Box"))
      .set_category(category)
      .set_animatable(false);
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
  if (menus.empty()) {
    return nullptr;
  }
  return std::move(menus.front());
}

void SelectPointsBaseTool::transform_objects(ObjectTransformation t)
{
  scene()->submit(m_transform_points_helper.make_command(t));
}

bool SelectPointsBaseTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  return mouse_press(pos, event, true);
}

bool SelectPointsBaseTool::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool allow_clear)
{
  const auto paths = type_cast<Path*>(scene()->template item_selection<Object>());
  if (AbstractSelectTool::mouse_press(pos, event)) {
    m_transform_points_helper.update(paths);
    return true;
  } else if (allow_clear && event.buttons() == Qt::LeftButton) {
    for (auto* path : paths) {
      for (auto&& point : *path) {
        point.is_selected = false;
      }
    }
    Q_EMIT scene()->mail_box().point_selection_changed();
    return false;
  } else {
    return false;
  }
}

bool SelectPointsBaseTool::has_transformation() const
{
  const auto paths = type_cast<Path*>(scene()->template item_selection<Object>());
  return std::any_of(paths.begin(), paths.end(), [](auto&& path) {
    return std::any_of(path->begin(), path->end(), [](auto&& point) { return point.is_selected; });
  });
}

BoundingBox SelectPointsBaseTool::bounding_box() const
{
  static const auto remove_tangents = [](const Point& point) { return point.nibbed(); };
  switch (property(BOUNDING_BOX_MODE_PROPERTY_KEY)->value<BoundingBoxMode>()) {
  case BoundingBoxMode::IncludeTangents:
    return BoundingBox(::transform<Point>(scene()->point_selection.points(Space::Viewport)));
  case BoundingBoxMode::ExcludeTangents:
    return BoundingBox(
        ::transform<Point>(scene()->point_selection.points(Space::Viewport), remove_tangents));
  case BoundingBoxMode::None:
    [[fallthrough]];
  default:
    return BoundingBox();
  }
}

void SelectPointsBaseTool::on_property_value_changed(Property* property)
{
  if (property == this->property(BOUNDING_BOX_MODE_PROPERTY_KEY)) {
    Q_EMIT scene()->mail_box().tool_appearance_changed(*this);
  }
  AbstractSelectTool::on_property_value_changed(property);
}

Vec2f SelectPointsBaseTool::selection_center() const
{
  return scene()->point_selection.center(Space::Viewport);
}

QString SelectPointsTool::type() const
{
  return TYPE;
}

void SelectPointsTool::reset()
{
  handles.clear();
  make_handles(*this, false);
  handles.push_back(std::make_unique<BoundingBoxHandle<SelectPointsTool>>(*this));
}

TransformPointsHelper::TransformPointsHelper(Scene& scene, Space space)
    : m_scene(scene), m_space(space)
{
  update();
  connect(&m_scene.mail_box(),
          &MailBox::point_selection_changed,
          this,
          qOverload<>(&TransformPointsHelper::update));
}

std::unique_ptr<PointsTransformationCommand>
TransformPointsHelper::make_command(const ObjectTransformation& t) const
{
  class TransformationCache : public Cache<Path*, ObjectTransformation>
  {
  public:
    TransformationCache(const Matrix& mat, Space space) : m_mat(mat), m_space(space)
    {
    }
    ObjectTransformation retrieve(Path* const& path) const override
    {
      const Matrix gt = path->global_transformation(m_space).to_mat();
      return ObjectTransformation(gt.inverted() * m_mat * gt);
    }

  private:
    const Matrix m_mat;
    const Space m_space;
  };

  assert(!t.has_nan());
  assert(!t.to_mat().has_nan());
  TransformationCache cache(t.to_mat(), m_space);

  PointsTransformationCommand::Map map;

  bool is_noop = true;
  for (auto&& [key, point] : m_initial_points) {
    const ObjectTransformation premul = cache.get(key.path);
    auto p = premul.apply(point);
    if (p.is_selected != point.is_selected) {
      p.is_selected = point.is_selected;
      is_noop = false;
    }
    map.insert(std::pair(key, p));
  }

  if (!is_noop) {
    return std::make_unique<PointsTransformationCommand>(map);
  } else {
    return nullptr;
  }
}

void TransformPointsHelper::update(const std::set<Path*>& paths)
{
  m_paths = paths;
  update();
}

void TransformPointsHelper::update()
{
  m_initial_points.clear();
  for (Path* path : m_paths) {
    for (Path::iterator it = path->begin(); it != path->end(); ++it) {
      if (it->is_selected) {
        m_paths.insert(path);
        m_initial_points.insert({it, *it});
      }
    }
  }
  Q_EMIT initial_transformations_changed();
}

}  // namespace omm
