#include "tools/selectpointstool.h"
#include "keybindings/keybindings.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "main/application.h"
#include "mainwindow/mainwindow.h"
#include "properties/optionproperty.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "scene/pointselection.h"
#include "tools/transformpointshelper.h"
#include <QMouseEvent>

namespace omm
{

SelectPointsBaseTool::SelectPointsBaseTool(Scene& scene)
    : AbstractSelectTool(scene)
    , m_transform_points_helper(std::make_unique<TransformPointsHelper>(scene, Space::Viewport))
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

SelectPointsBaseTool::~SelectPointsBaseTool() = default;

PointSelectHandle::TangentMode SelectPointsBaseTool::tangent_mode() const
{
  const auto i = property(TANGENT_MODE_PROPERTY_KEY)->value<std::size_t>();
  return static_cast<PointSelectHandle::TangentMode>(i);
}

std::unique_ptr<QMenu> SelectPointsBaseTool::make_context_menu(QWidget* parent)
{
  Q_UNUSED(parent)
  auto& app = Application::instance();
  auto menus = app.key_bindings->make_menus(app, MainWindow::path_menu_entries());
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
  scene()->submit(m_transform_points_helper->make_command(t));
}

bool SelectPointsBaseTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  return mouse_press(pos, event, true);
}

bool SelectPointsBaseTool::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool allow_clear)
{
  const auto path_objects = type_casts<PathObject*>(scene()->template item_selection<Object>());
  if (AbstractSelectTool::mouse_press(pos, event)) {
    m_transform_points_helper->update(path_objects);
    return true;
  } else if (allow_clear && event.buttons() == Qt::LeftButton) {
    for (auto* path_object : path_objects) {
      path_object->geometry().deselect_all_points();
    }
    Q_EMIT scene()->mail_box().point_selection_changed();
    return false;
  } else {
    return false;
  }
}

void SelectPointsBaseTool::make_handles()
{
  for (auto* path_object : scene()->item_selection<PathObject>()) {
    const auto points = path_object->geometry().points();
    for (auto* point : points) {
      auto handle = std::make_unique<PointSelectHandle>(*this, *path_object, *point);
      push_handle(std::move(handle));
    }
  }
}

BoundingBox SelectPointsBaseTool::bounding_box() const
{
  static const auto remove_tangents = [](const Point& point) { return point.nibbed(); };
  switch (property(BOUNDING_BOX_MODE_PROPERTY_KEY)->value<BoundingBoxMode>()) {
  case BoundingBoxMode::IncludeTangents:
    return BoundingBox(scene()->point_selection->points(Space::Viewport));
  case BoundingBoxMode::ExcludeTangents:
    return BoundingBox{util::transform(scene()->point_selection->points(Space::Viewport), remove_tangents)};
  case BoundingBoxMode::None:
    [[fallthrough]];
  default:
    return BoundingBox{};
  }
}

void SelectPointsBaseTool::on_property_value_changed(Property* property)
{
  if (property == this->property(BOUNDING_BOX_MODE_PROPERTY_KEY)) {
    Q_EMIT scene()->mail_box().tool_appearance_changed(*this);
  }
  AbstractSelectTool::on_property_value_changed(property);
}

SceneMode SelectPointsBaseTool::scene_mode() const
{
  return SceneMode::Vertex;
}

void SelectPointsBaseTool::reset()
{
  clear();
  make_handles();
}

Vec2f SelectPointsBaseTool::selection_center() const
{
  return scene()->point_selection->center(Space::Viewport);
}

}  // namespace omm
