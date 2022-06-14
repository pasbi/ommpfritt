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

namespace
{

template<typename T, typename... Ts> void emplace_back(auto& cs, T&& arg, Ts&&... args)
{
  cs.emplace_back(std::forward<T>(arg));
  if constexpr (sizeof...(args) > 0) {
    emplace_back(cs, std::forward<Ts>(args)...);
  }
}

template<template<typename...> typename Container, typename... Args> decltype(auto) make(Args&&... args)
{
  using T = std::tuple_element_t<0, std::tuple<std::decay_t<Args>...>>;
  Container<T> cs;
  if constexpr (requires(Container<T> cs, std::size_t n) { cs.reserve(n); }) {
    cs.reserve(sizeof...(args));
  }
  emplace_back(cs, std::forward<Args>(args)...);
  return cs;
}

}  // namespace

namespace omm
{

class PathTool::PathBuilder
{
public:
  explicit PathBuilder(Scene& scene) : m_scene(scene) {}

  /**
   * @brief create_first_path_point a new path is created, not connected to any existing paths.
   * @param pos the geometry of the first point
   */
  void create_first_path_point(const Point& pos)
  {
    assert(is_valid());
    m_current_path = &current_path_vector().add_path();
    std::deque<OwnedLocatedPath> owlps;
    auto point = std::make_shared<PathPoint>(pos, &current_path_vector());
    m_current_point = point.get();
    owlps.emplace_back(m_current_path, 0, std::deque{std::move(point)});
    m_scene.submit<AddPointsCommand>(std::move(owlps), current_path_vector().path_object());
    assert(is_valid());
  }

  void submit_add_points_command(omm::PathObject& path_object,
                                 omm::Path& path,
                                 const std::size_t point_offset,
                                 std::deque<std::shared_ptr<PathPoint>>&& points)
  {
    std::deque<OwnedLocatedPath> owlps;
    owlps.emplace_back(&path, point_offset, std::move(points));
    auto command = std::make_unique<AddPointsCommand>(std::move(owlps), &path_object);
    const auto new_edges = command->new_edges();
    m_scene.submit(std::move(command));
    if (!new_edges.empty()) {
      m_last_point = new_edges.back()->b().get();
    }
  }

  void add_point(Point point)
  {
    ensure_active_path_object();
    assert(is_valid());
    point = m_current_path_object->global_transformation(Space::Viewport).inverted().apply(point);
    if (m_last_point == nullptr) {
      create_first_path_point(point);
    } else {
      auto b = std::make_shared<PathPoint>(point, &current_path_vector());
      m_current_point = b.get();
      if (m_last_point == m_current_path->last_point().get()) {
        // append
        submit_add_points_command(*m_current_path_object, *m_current_path, m_current_path->points().size(), {b});
      } else {
        // branch
        assert(m_last_point != nullptr);
        auto root = current_path_vector().share(*m_last_point);
        m_current_path = &current_path_vector().add_path();
        submit_add_points_command(*m_current_path_object, *m_current_path, 0, {root, b});
      }
    }
    assert(is_valid());
  }

  void find_tie()
  {
    if (const auto selected_paths = m_scene.item_selection<PathObject>(); selected_paths.empty()) {
      assert(is_valid());
      return;
    } else {
      assert(is_valid());
      m_current_path_object = *selected_paths.begin();
      if (const auto ps = m_current_path_object->path_vector().selected_points(); ps.empty()) {
        m_current_path_object = nullptr;
      } else {
        m_current_point = *ps.begin();
        m_last_edge = nullptr;
      }
      assert(is_valid());
    }
  }

  void ensure_active_path_object()
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

  bool move_tangents(const Vec2f& delta)
  {
    if (m_current_path == nullptr || m_current_point == nullptr) {
      return false;
    }

    const auto lt = PolarCoordinates(m_current_point->geometry().left_tangent().to_cartesian() + delta);
    auto geometry = m_current_point->geometry();
    geometry.set_left_tangent(lt);
    geometry.set_right_tangent(-lt);
    m_current_point->set_geometry(geometry);
    m_current_path_object->update();
    return true;
  }

  void release()
  {
    m_last_point = m_current_point;
    m_current_point = nullptr;
    m_macro.reset();
  }

  void end()
  {
    if (m_current_path_object != nullptr) {
      m_current_path_object->property(PathObject::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
    }
  }

  PathVector& current_path_vector() const
  {
    assert(m_current_path_object != nullptr);
    return m_current_path_object->path_vector();
  }

  bool is_valid() const
  {
    if (!m_current_path_object) {
      return true;
    }
    const auto& paths = m_current_path_object->path_vector().paths();
    return std::all_of(paths.begin(), paths.end(), [](const Path* p) { return p->is_valid(); });
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
  } else {
    return m_path_builder->move_tangents(delta);
  }
}

bool PathTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  const auto control_modifier = static_cast<bool>(event.modifiers() & Qt::ControlModifier);
  if (!control_modifier && SelectPointsBaseTool::mouse_press(pos, event, false)) {
    m_path_builder->find_tie();
    return true;
  } else {
    if (event.button() == Qt::LeftButton) {
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
//  m_path_builder->find_tie();
  SelectPointsBaseTool::reset();
}

void PathTool::draw(Painter& painter) const
{
  SelectPointsBaseTool::draw(painter);
}

}  // namespace omm
