#include "tools/pathtool.h"
#include "commands/addcommand.h"
#include "commands/modifypointscommand.h"
#include "commands/joinpointscommand.h"
#include "main/application.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "scene/scene.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "tools/selecttool.h"
#include "tools/handles/handle.h"
#include <QMouseEvent>

namespace omm
{

struct PathTool::Current
{
  PathObject* path_object = nullptr;
  Path* path = nullptr;
  PathPoint* point = nullptr;
  PathPoint* last_point = nullptr;

  void find_tie(const Scene& scene)
  {
    path_object = nullptr;
    path = nullptr;
    last_point = nullptr;
    auto path_objects = type_casts<PathObject*>(scene.item_selection<Object>());
    if (path_objects.size() == 1) {
      path_object = *path_objects.begin();
      if (auto sp = path_object->geometry().selected_points(); !sp.empty()) {
        last_point = sp.front();
        path = path_object->geometry().find_path(*last_point);
      }
    } else {
      path_object = nullptr;
    }
  }
};

}  // namespace omm

namespace
{

using namespace omm;

class LeftButtonPressImpl
{
public:
  explicit LeftButtonPressImpl(Scene& scene, PathTool::Current& current)
    : m_scene(scene)
    , m_current(current)
  {
  }

  void find_target_point(Tool& tool, const Vec2f& pos)
  {
    for (auto* handle : tool.handles()) {
      const auto* point_select_handle = dynamic_cast<PointSelectHandle*>(handle);
      if (point_select_handle != nullptr && point_select_handle->contains_global(pos)) {
        m_target_point = &point_select_handle->point();
        return;
      }
    }
  }

  void insert_point_segment(const Point& point, const std::size_t index)
  {
    std::deque<std::unique_ptr<PathPoint>> points;
    m_current.point = points.emplace_back(std::make_unique<PathPoint>(point, *m_current.path)).get();
    m_located_paths.emplace_back(m_current.path, index, std::move(points));
    if (m_target_point != nullptr) {
      m_points_to_join.insert({m_current.point, m_target_point});
    }
  }

  void add_point(const Point& point)
  {
    if (m_current.path == nullptr) {
      // no path is selected: add the point to a newly created segment
      auto new_path = std::make_unique<Path>(std::deque{point}, &m_current.path_object->geometry());
      m_current.point = &new_path->at(0);
      m_located_paths.emplace_back(std::move(new_path));
    } else if (m_current.path->size() == 0 || m_current.path->points().back() == m_current.last_point) {
      // segment is empty or last point of the segmet is selected: append point at end
      insert_point_segment(point, m_current.path->size());
    } else if (m_current.path->points().front() == m_current.last_point) {
      // first point of segment is selected: append point at begin
      insert_point_segment(point, 0);
    } else {
      // other point of segment is selected: add point to a newly created segment and join points
      auto new_path = std::make_unique<Path>(std::deque{m_current.last_point->geometry(), point},
                                             &m_current.path_object->geometry());
      m_current.point = &new_path->at(1);
      m_points_to_join = {&new_path->at(0), m_current.last_point};
      m_located_paths.emplace_back(std::move(new_path));
    }
  }

  void polish()
  {
    PathObject& current_path = *m_current.point->path_vector()->path_object();
    if (!m_points_to_join.empty()) {
      start_macro();
    }
    m_scene.submit<AddPointsCommand>(current_path, std::move(m_located_paths));
    if (!m_points_to_join.empty()) {
      m_scene.submit<JoinPointsCommand>(m_scene, std::deque{m_points_to_join});
    }
    current_path.geometry().deselect_all_points();
    m_current.point->set_selected(true);
    current_path.update();
  }

  void ensure_active_path(Scene& scene, PathTool::Current& current)
  {
    if (current.path_object == nullptr) {
      start_macro();
      static constexpr auto insert_mode = Application::InsertionMode::Default;
      auto& path_object = Application::instance().insert_object(PathObject::TYPE, insert_mode);
      current.path_object = dynamic_cast<PathObject*>(&path_object);
      current.path_object->property(PathObject::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
      scene.set_selection({current.path_object});
    }
  }

  void start_macro()
  {
    if (m_macro == nullptr) {
      m_macro = m_scene.history().start_macro(AddPointsCommand::static_label());
    }
  }

private:
  Scene& m_scene;
  PathTool::Current& m_current;
  std::unique_ptr<Macro> m_macro;
  PathPoint* m_target_point = nullptr;
  std::deque<AddPointsCommand::OwnedLocatedPath> m_located_paths;
  ::transparent_set<PathPoint*> m_points_to_join;
};

}  // namespace

namespace omm
{

PathTool::PathTool(Scene& scene)
  : SelectPointsBaseTool(scene)
  , m_current(std::make_unique<Current>())
{
}

PathTool::~PathTool() = default;

bool PathTool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event)
{
  if (SelectPointsBaseTool::mouse_move(delta, pos, event)) {
    return true;
  } else if (m_current->path == nullptr || m_current->point ==  nullptr) {
    return false;
  } else {
    const auto lt = PolarCoordinates(m_current->point->geometry().left_tangent().to_cartesian() + delta);
    auto geometry = m_current->point->geometry();
    geometry.set_left_tangent(lt);
    geometry.set_right_tangent(-lt);
    m_current->point->set_geometry(geometry);
    m_current->path_object->update();
    return true;
  }
}

bool PathTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  const auto control_modifier = static_cast<bool>(event.modifiers() & Qt::ControlModifier);
  if (!control_modifier && SelectPointsBaseTool::mouse_press(pos, event, false)) {
    return true;
  } else {
    m_current->find_tie(*scene());
    if (event.button() == Qt::LeftButton) {
      LeftButtonPressImpl impl{*scene(), *m_current};
      if (control_modifier) {
        impl.find_target_point(*this, pos);
      }
      impl.ensure_active_path(*scene(), *m_current);
      const auto transformation = m_current->path_object->global_transformation(Space::Viewport).inverted();
      const Point point{transformation.apply_to_position(pos)};
      impl.add_point(point);
      impl.polish();
      return true;
    }
    return false;
  }
}

void PathTool::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  SelectPointsBaseTool::mouse_release(pos, event);
  m_current->point = nullptr;
}

QString PathTool::type() const
{
  return TYPE;
}

void PathTool::end()
{
  SelectPointsBaseTool::end();
  if (m_current->path != nullptr) {
    m_current->path_object->property(PathObject::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
  }
}

void PathTool::reset()
{
  m_current->find_tie(*scene());
  SelectPointsBaseTool::reset();
}

}  // namespace omm
