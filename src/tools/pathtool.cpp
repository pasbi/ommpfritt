#include "tools/pathtool.h"
#include "scene/scene.h"
#include "tools/selecttool.h"
#include <QMouseEvent>
#include "commands/addcommand.h"
#include <mainwindow/application.h>
#include "commands/modifypointscommand.h"

namespace omm
{

PathTool::PathTool(Scene &scene) : SelectPointsTool(scene) {  }

bool PathTool::mouse_move(const Vec2f &delta, const Vec2f &pos, const QMouseEvent &event)
{
  if (SelectPointsTool::mouse_move(delta, pos, event)) {
      return true;
  } else if (m_path != nullptr && m_current_point != nullptr) {
    const auto lt = PolarCoordinates(m_current_point->left_tangent.to_cartesian() - delta);
    m_current_point->left_tangent = lt;
    m_current_point->right_tangent = -lt;
    m_path->update();
    return true;
  } else {
    return false;
  }
}

bool PathTool::mouse_press(const Vec2f &pos, const QMouseEvent &event, bool force)
{
  if (SelectPointsTool::mouse_press(pos, event, force)) {
    return true;
  } else {
    switch (event.button()) {
    case Qt::LeftButton:
      add_point(pos);
      break;
    case Qt::RightButton:
      end();
      break;
    default:
      break;
    }
    return false;
  }
}

void PathTool::mouse_release(const Vec2f &pos, const QMouseEvent &event)
{
  SelectPointsTool::mouse_release(pos, event);
  m_current_point = nullptr;
}

std::string PathTool::type() const { return TYPE; }

void PathTool::add_point(const Vec2f &pos)
{
  if (!m_path) {
    const auto insert_mode = Application::InsertionMode::Default;
    m_path = static_cast<Path*>(&Application::instance().insert_object(Path::TYPE, insert_mode));
    m_path->property(Path::INTERPOLATION_PROPERTY_KEY)->set(Path::InterpolationMode::Bezier);
    scene.set_selection({m_path});
  }

  const auto gpos = viewport_transformation.inverted().apply_to_position(pos);

  Path::PointSequence point_sequence(m_path->points().size(), { Point(gpos) });

  scene.submit<AddPointsCommand>(std::map{ std::pair{ m_path, std::vector{ point_sequence } } });
  m_current_point = m_path->points_ref().back();
  m_path->update();
  on_scene_changed();
}

void PathTool::end()
{
  SelectPointsTool::end();
  if (m_path != nullptr) {
    m_path->property(Path::INTERPOLATION_PROPERTY_KEY)->set(Path::InterpolationMode::Bezier);
  }
}

void PathTool::on_scene_changed()
{
  auto paths = type_cast<Path*>(scene.item_selection<Object>());
  if (paths.size() == 1) {
    m_path = *paths.begin();
  } else {
    m_path = nullptr;
  }

  handles.clear();
  SelectPointsTool::make_handles(*this, true);
}

}  // namespace
