#include "tools/pathtool.h"
#include "commands/addcommand.h"
#include "commands/addremovepointscommand.h"
#include "commands/joinpointscommand.h"
#include "commands/modifypointscommand.h"
#include "main/application.h"
#include "objects/pathobject.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "renderers/painter.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/scene.h"
#include "tools/handles/handle.h"
#include "tools/selecttool.h"
#include <QMouseEvent>

namespace omm
{

//  void find_tie(const Scene& scene)
//  {
//    path_object = nullptr;
//    path = nullptr;
//    last_point = nullptr;
//    auto path_objects = type_casts<PathObject*>(scene.item_selection<Object>());
//    if (path_objects.size() == 1) {
//      path_object = *path_objects.begin();
//      if (auto sp = path_object->geometry().selected_points(); !sp.empty()) {
//        last_point = sp.front();
//        path = path_object->geometry().find_path(*last_point);
//      }
//    } else {
//      path_object = nullptr;
//    }
//  }

}  // namespace omm

namespace omm
{

class PathTool::PathBuilder
{
public:
  explicit PathBuilder(Scene& scene) : m_scene(scene) {}

  void add_point(Point point)
  {
    point = m_current_path_object->global_transformation(Space::Viewport).inverted().apply(point);
    auto& pv = m_current_path_object->path_vector();
    if (m_last_point == nullptr) {
      m_current_path = &pv.add_path();
      m_first_point = std::make_unique<PathPoint>(point, &pv);
      m_current_point = m_first_point.get();
    } else {
      assert((m_last_edge == nullptr) != (m_first_point == nullptr));
      auto a = m_first_point ? std::move(m_first_point) : m_last_edge->b();
      auto b = std::make_unique<PathPoint>(point, &pv);
      m_current_point = b.get();
      m_last_edge = &m_current_path->add_edge(a, std::move(b));
    }
    m_current_path_object->update();
  }

  void find_tie()
  {

  }

  void ensure_active_path()
  {
    if (m_current_path_object == nullptr) {
      start_macro();
      static constexpr auto insert_mode = Application::InsertionMode::Default;
      auto& path_object = Application::instance().insert_object(PathObject::TYPE, insert_mode);
      m_current_path_object = dynamic_cast<PathObject*>(&path_object);
      m_current_path_object->property(PathObject::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
      m_scene.set_selection({m_current_path_object});
    }
  }

  bool has_active_path_object() const
  {
    return m_current_path_object != nullptr;
  }

  bool has_active_path() const
  {
    return m_current_path != nullptr;
  }

  bool has_active_point() const
  {
    return m_current_point != nullptr;
  }

  bool is_floating() const
  {
    return m_first_point != nullptr;
  }

  void release()
  {
    m_last_point = m_current_point;
    m_current_point = nullptr;
  }

  void end()
  {
    if (m_current_path != nullptr) {
      m_current_path_object->property(PathObject::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
    }
  }

private:
  void start_macro()
  {
    if (m_macro == nullptr) {
      m_macro = m_scene.history().start_macro(AddPointsCommand::static_label());
    }
  }

  PathObject* m_current_path_object = nullptr;
  Path* m_current_path = nullptr;
  PathPoint* m_last_point = nullptr;
  PathPoint* m_current_point = nullptr;
  std::unique_ptr<PathPoint> m_first_point;
  Edge* m_last_edge;
  Scene& m_scene;
  std::unique_ptr<Macro> m_macro;
};

//class LeftButtonPressImpl
//{
//public:
//  explicit LeftButtonPressImpl(Scene& scene, PathTool::Current& current)
//    : m_scene(scene)
//    , m_current(current)
//  {
//  }

////  void find_target_point(Tool& tool, const Vec2f& pos)
////  {
////    for (auto* handle : tool.handles()) {
////      const auto* point_select_handle = dynamic_cast<PointSelectHandle*>(handle);
////      if (point_select_handle != nullptr && point_select_handle->contains_global(pos)) {
////        m_target_point = &point_select_handle->point();
////        return;
////      }
////    }
////  }

////  void insert_point_segment(const Point& point, const std::size_t index)
////  {
////    std::deque<std::unique_ptr<PathPoint>> points;
////    m_current.point = points.emplace_back(std::make_unique<PathPoint>(point, m_current.path->path_vector())).get();
////    m_located_paths.emplace_back(m_current.path, index, std::move(points));
////    if (m_target_point != nullptr) {
////      m_points_to_join.insert({m_current.point, m_target_point});
////    }
////  }

//  void add_point(const Point& point)
//  {
//  }

//  void polish()
//  {
//    PathObject& current_path = *m_current.point->path_vector()->path_object();
//    if (!m_points_to_join.empty()) {
//      start_macro();
//    }
//    m_scene.submit<AddPointsCommand>(std::move(m_located_paths));
//    current_path.geometry().deselect_all_points();
//    m_current.point->set_selected(true);
//    current_path.update();
//  }


//  void start_macro()
//  {
//    if (m_macro == nullptr) {
//      m_macro = m_scene.history().start_macro(AddPointsCommand::static_label());
//    }
//  }

//private:
//  PathTool::Current& m_current;
////  PathPoint* m_target_point = nullptr;
//  std::deque<OwnedLocatedPath> m_located_paths;
//  ::transparent_set<PathPoint*> m_points_to_join;
//};

PathTool::PathTool(Scene& scene)
  : SelectPointsBaseTool(scene)
  , m_path_builder(std::make_unique<PathBuilder>(scene))
{
}

PathTool::~PathTool() = default;

bool PathTool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event)
{
  if (SelectPointsBaseTool::mouse_move(delta, pos, event)) {
    return true;
  } else if (!m_path_builder->has_active_path() || !m_path_builder->has_active_point()) {
    return false;
  } else {
//    const auto lt = PolarCoordinates(m_current->point->geometry().left_tangent().to_cartesian() + delta);
//    auto geometry = m_current->point->geometry();
//    geometry.set_left_tangent(lt);
//    geometry.set_right_tangent(-lt);
//    m_current->point->set_geometry(geometry);
//    m_current->path_object->update();
    return true;
  }
}

bool PathTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  const auto control_modifier = static_cast<bool>(event.modifiers() & Qt::ControlModifier);
  if (!control_modifier && SelectPointsBaseTool::mouse_press(pos, event, false)) {
    return true;
  } else {
    m_path_builder->find_tie();
    if (event.button() == Qt::LeftButton) {
      m_path_builder->ensure_active_path();
      m_path_builder->add_point(Point{pos});
      reset();
      return true;
    }
    return false;
  }
}

void PathTool::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  SelectPointsBaseTool::mouse_release(pos, event);
  m_path_builder->release();
}

QString PathTool::type() const
{
  return TYPE;
}

void PathTool::end()
{
  SelectPointsBaseTool::end();
  m_path_builder->end();
}

void PathTool::reset()
{
  m_path_builder->find_tie();
  SelectPointsBaseTool::reset();
}

void PathTool::draw(Painter& painter) const
{
  SelectPointsBaseTool::draw(painter);
  if (m_path_builder->is_floating()) {
//    const auto pos = transformation().apply_to_position(m_current->first_point->geometry().position());
//    const auto pos = m_current->first_point->geometry().position();
    painter.painter->fillRect(centered_rectangle({0, 0}, 10), Qt::red);
  }
}

}  // namespace omm
