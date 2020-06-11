#include "tools/pathtool.h"
#include "scene/scene.h"
#include "tools/selecttool.h"
#include <QMouseEvent>
#include "commands/addcommand.h"
#include <mainwindow/application.h>
#include "commands/modifypointscommand.h"

namespace omm
{

PathTool::PathTool(Scene &scene) : SelectPointsBaseTool(scene) {  }

bool PathTool::mouse_move(const Vec2f &delta, const Vec2f &pos, const QMouseEvent &event)
{
  if (SelectPointsBaseTool::mouse_move(delta, pos, event)) {
      return true;
  } else if (m_path != nullptr && m_current_point) {
    const auto lt = PolarCoordinates((*m_current_point)->left_tangent.to_cartesian() - delta);
    (*m_current_point)->left_tangent = lt;
    (*m_current_point)->right_tangent = -lt;
    m_path->update();
    return true;
  } else {
    return false;
  }
}

bool PathTool::mouse_press(const Vec2f &pos, const QMouseEvent &event)
{
  if (SelectPointsBaseTool::mouse_press(pos, event, false)) {
    return true;
  } else {
    if (event.button() == Qt::LeftButton) {
      m_current_point = add_point(pos);
    }
    return false;
  }
}

void PathTool::mouse_release(const Vec2f &pos, const QMouseEvent &event)
{
  SelectPointsBaseTool::mouse_release(pos, event);
  m_current_point.reset();
}

QString PathTool::type() const { return TYPE; }

Path::iterator PathTool::add_point(const Vec2f &pos)
{
  Q_UNUSED(pos)
  if (!m_path) {
    const auto insert_mode = Application::InsertionMode::Default;
    m_path = static_cast<Path*>(&Application::instance().insert_object(Path::TYPE, insert_mode));
    m_path->property(Path::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
    scene()->set_selection({m_path});
  }

  const auto gpos = m_path->global_transformation(Space::Viewport).inverted().apply_to_position(pos);
  static const auto is_selected = [](const auto& point) {
    return point.is_selected;
  };

  const auto link = [this]() {
    const auto first_selected = std::find_if(m_path->begin(), m_path->end(), is_selected);
    if (first_selected != m_path->end()) {
      const auto n_selected_points = std::count_if(m_path->begin(), m_path->end(), is_selected);
      if (n_selected_points == 1) {
        if (first_selected.point == 0) {
          return first_selected;
        } else if (first_selected.point + 1 == m_path->segments[first_selected.segment].size()) {
          return Path::iterator{*m_path, first_selected.segment, first_selected.point + 1};
        }
      }
    }
    return m_path->end();
  }();

  const AddPointsCommand::LocatedSegment segment{link, {Point(gpos)}};
  scene()->submit<AddPointsCommand>(*m_path, std::vector{segment});

  for (Point& p : *m_path) {
    p.is_selected = false;
  }
  (*link).is_selected = true;

  m_path->update();
  reset();
  return link;
}

void PathTool::end()
{
  SelectPointsBaseTool::end();
  if (m_path != nullptr) {
    m_path->property(Path::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
  }
}

void PathTool::reset()
{
  auto paths = type_cast<Path*>(scene()->item_selection<Object>());
  if (paths.size() == 1) {
    m_path = *paths.begin();
  } else {
    m_path = nullptr;
  }

  handles.clear();
  SelectPointsTool::make_handles(*this, true);
}

}  // namespace
