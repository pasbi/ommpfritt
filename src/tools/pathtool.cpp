#include "tools/pathtool.h"
#include "commands/addcommand.h"
#include "commands/modifypointscommand.h"
#include "main/application.h"
#include "objects/path.h"
#include "objects/segment.h"
#include "scene/scene.h"
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
    const auto lt = PolarCoordinates(m_current_point->left_tangent.to_cartesian() + delta);
    m_current_point->left_tangent = lt;
    m_current_point->right_tangent = -lt;
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
    if (m_current_path == nullptr) {
      auto new_path = std::make_unique<Path>(scene());
      m_current_path = new_path.get();
      scene()->object_tree().root().adopt(std::move(new_path));
    }
    if (event.button() == Qt::LeftButton) {
      const auto index =  m_current_segment == nullptr ? 0 : m_current_segment->size();
      std::deque<std::unique_ptr<Point>> points;
      m_current_point = points.emplace_back(std::make_unique<Point>(pos)).get();
      std::deque<AddPointsCommand::OwnedLocatedSegment> located_segments;
      located_segments.emplace_back(AddPointsCommand::OwnedLocatedSegment{m_current_segment, index, std::move(points)});
      scene()->submit<AddPointsCommand>(*m_current_path, std::move(located_segments));
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

//PathIterator PathTool::add_point(const Vec2f& pos)
//{
//  Q_UNUSED(pos)
//  if (m_path == nullptr) {
//    const auto insert_mode = Application::InsertionMode::Default;
//    m_path = dynamic_cast<Path*>(&Application::instance().insert_object(Path::TYPE, insert_mode));
//    m_path->property(Path::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
//    scene()->set_selection({m_path});
//  }

//  const auto gpos
//      = m_path->global_transformation(Space::Viewport).inverted().apply_to_position(pos);
//  static const auto is_selected = [](const auto& point) { return point.is_selected; };

//  const auto link = [this]() {
//    const auto first_selected = std::find_if(m_path->begin(), m_path->end(), is_selected);
//    if (first_selected != m_path->end()) {
//      const auto n_selected_points = std::count_if(m_path->begin(), m_path->end(), is_selected);
//      if (n_selected_points == 1) {
//        if (first_selected.point == 0) {
//          return first_selected;
//        } else if (first_selected.point + 1 == m_path->segments[first_selected.segment].size()) {
//          return PathIterator{*m_path, first_selected.segment, first_selected.point + 1};
//        }
//      }
//    }
//    return m_path->end();
//  }();

//  const AddPointsCommand::LocatedSegment segment{link, {Point(gpos)}};
//  scene()->submit<AddPointsCommand>(*m_path, std::vector{segment});

//  for (Point& p : *m_path) {
//    p.is_selected = false;
//  }
//  (*link).is_selected = true;

//  m_path->update();
//  reset();
//  return link;
//}

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
