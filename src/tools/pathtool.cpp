#include "tools/pathtool.h"
#include "commands/addcommand.h"
#include "commands/modifypointscommand.h"
#include "commands/joinpointscommand.h"
#include "main/application.h"
#include "objects/path.h"
#include "objects/pathpoint.h"
#include "objects/segment.h"
#include "scene/scene.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "tools/selecttool.h"
#include <QMouseEvent>

namespace omm
{

PathTool::PathTool(Scene& scene) : SelectPointsBaseTool(scene)
{
}

bool PathTool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event)
{
  if (SelectPointsBaseTool::mouse_move(delta, pos, event)) {
    return true;
  } else if (m_current_path == nullptr || m_current_point ==  nullptr) {
    return false;
  } else {
    const auto lt = PolarCoordinates(m_current_point->geometry().left_tangent().to_cartesian() + delta);
    m_current_point->geometry().set_left_tangent(lt);
    m_current_point->geometry().set_right_tangent(-lt);
    m_current_path->update();
    return true;
  }
}

bool PathTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  if (SelectPointsBaseTool::mouse_press(pos, event, false)) {
    return true;
  } else {
    find_tie();
    if (event.button() == Qt::LeftButton) {
      std::unique_ptr<Macro> macro;
      if (m_current_path == nullptr) {
        macro = scene()->history().start_macro(AddPointsCommand::static_label());
        static constexpr auto insert_mode = Application::InsertionMode::Default;
        m_current_path = dynamic_cast<Path*>(&Application::instance().insert_object(Path::TYPE, insert_mode));
        m_current_path->property(Path::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
        scene()->set_selection({m_current_path});
      }
      const auto transformation = m_current_path->global_transformation(Space::Viewport).inverted();
      const auto gpos = transformation.apply_to_position(pos);

      std::deque<std::unique_ptr<PathPoint>> points;
      JoinPointsCommand::Map join_points_map;
      m_current_point = points.emplace_back(std::make_unique<PathPoint>(Point{gpos})).get();
      std::deque<AddPointsCommand::OwnedLocatedSegment> located_segments;
      if (m_current_segment == nullptr) {
        // no segment is selected: add the point to a newly created segment
        located_segments.emplace_back(std::make_unique<Segment>(std::move(points)));
      } else if (m_current_segment->size() == 0 || m_current_segment->points().back() == m_last_point) {
        // segment is empty or last point of the segmet is selected: append point at end
        located_segments.emplace_back(m_current_segment, m_current_segment->size(), std::move(points));
      } else if (m_current_segment->points().front() == m_last_point) {
        // first point of segment is selected: append point at begin
        located_segments.emplace_back(m_current_segment, 0, std::move(points));
      } else {
        // other point of segment is selected: add point to a newly created segment and join points
        auto& copy = *points.emplace_front(std::make_unique<PathPoint>(m_last_point->geometry()));
        located_segments.emplace_back(std::make_unique<Segment>(std::move(points)));
        join_points_map[m_current_path].insert({m_last_point, &copy});
      }
      scene()->submit<AddPointsCommand>(*m_current_path, std::move(located_segments));
      if (!join_points_map.empty()) {
        scene()->submit<JoinPointsCommand>(join_points_map);
      }
      m_current_path->deselect_all_points();
      m_current_point->set_selected(true);
      m_current_path->update();
      return true;
    }
    return false;
  }
}

void PathTool::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  SelectPointsBaseTool::mouse_release(pos, event);
  m_current_point = nullptr;
}

QString PathTool::type() const
{
  return TYPE;
}

void PathTool::end()
{
  SelectPointsBaseTool::end();
  if (m_current_path != nullptr) {
    m_current_path->property(Path::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
  }
}

void PathTool::reset()
{
  find_tie();
  handles.clear();
  SelectPointsTool::make_handles(*this, true);
}

void PathTool::find_tie()
{
  m_current_path = nullptr;
  m_current_segment = nullptr;
  m_last_point = nullptr;
  auto paths = type_casts<Path*>(scene()->item_selection<Object>());
  if (paths.size() == 1) {
    m_current_path = *paths.begin();
    if (auto sp = m_current_path->selected_points(); !sp.empty()) {
      m_last_point = sp.front();
      m_current_segment = m_current_path->find_segment(*m_last_point);
    }
  } else {
    m_current_path = nullptr;
  }
}

}  // namespace omm
