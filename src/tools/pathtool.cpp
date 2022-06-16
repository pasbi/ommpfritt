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
    auto point = std::make_shared<PathPoint>(pos, &current_path_vector());
    m_current_point = point.get();
    OwnedLocatedPath olp(m_current_path, 0, std::deque{std::move(point)});
    m_scene.submit<AddPointsCommand>(std::move(olp), current_path_vector().path_object());
    assert(is_valid());
  }

  void submit_add_points_command(omm::PathObject& path_object,
                                 omm::Path& path,
                                 const std::size_t point_offset,
                                 std::deque<std::shared_ptr<PathPoint>>&& points)
  {
    OwnedLocatedPath olp(&path, point_offset, std::move(points));
    auto command = std::make_unique<AddPointsCommand>(std::move(olp), &path_object);
    const auto new_edges = command->new_edges();
    m_scene.submit(std::move(command));
    if (!new_edges.empty()) {
      m_last_point = new_edges.back()->b().get();
    }
  }

  void add_point(std::shared_ptr<PathPoint>&& b)
  {
    m_current_point = b.get();
    if (m_last_point == m_current_path->last_point().get()) {
      // append
      submit_add_points_command(*m_current_path_object, *m_current_path, m_current_path->points().size(), {b});
    } else if (auto root = current_path_vector().share(*m_last_point); root != nullptr) {
      // branch
      assert(m_last_point != nullptr);
      m_current_path = &current_path_vector().add_path();
      submit_add_points_command(*m_current_path_object, *m_current_path, 0, {root, b});
    } else {
      // m_last_point has been removed (e.g. by undo), also append.
      submit_add_points_command(*m_current_path_object, *m_current_path, m_current_path->points().size(), {b});
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
      add_point(std::make_shared<PathPoint>(point, &current_path_vector()));
    }
    assert(is_valid());
  }

  void find_tie()
  {
    assert(is_valid());
    m_current_point = nullptr;
    if (const auto selected_paths = m_scene.item_selection<PathObject>(); selected_paths.empty()) {
      m_current_path_object = nullptr;
    } else {
      m_current_path_object = *selected_paths.begin();
      if (const auto ps = m_current_path_object->path_vector().selected_points(); !ps.empty()) {
        m_current_point = *ps.begin();
      }
    }
    assert(is_valid());
  }

  void close_path()
  {
    if (const auto selection = current_path_vector().selected_points(); m_last_point != nullptr && !selection.empty()) {
      if (auto end = current_path_vector().share(**selection.begin()); end) {
        // it may be that start and end have been removed from the path in the meanwhile (undo)
        add_point(std::move(end));
      }
    }
  }

  PathObject* find_selected_path_object() const
  {
    const auto selection = m_scene.selection();
    static constexpr auto is_path_object = [](const auto* item) { return item->type() == PathObject::TYPE; };
    const auto it = std::find_if(selection.begin(), selection.end(), is_path_object);
    if (it == selection.end()) {
      return nullptr;
    } else {
      return dynamic_cast<PathObject*>(*it);
    }
  }

  void ensure_active_path_object()
  {
    if (m_scene.contains(m_current_path_object) && m_scene.selection().contains(m_current_path_object)) {
      // everything can stay as it is, we have an existing and selected m_current_path_object.
      return;
    } else if (auto* const selected_path_object = find_selected_path_object(); selected_path_object != nullptr) {
      // There is a path object selected, but it's not m_current_path_object.
      // Use the selected path object.
      m_current_path_object = selected_path_object;
    } else {
      // There is no path object selected and m_current_path_object doesn't exist currently.
      // Create a new one.
      start_macro();
      static constexpr auto insert_mode = Application::InsertionMode::Default;
      auto& path_object = Application::instance().insert_object(PathObject::TYPE, insert_mode);
      m_current_path_object = dynamic_cast<PathObject*>(&path_object);
      m_current_path_object->property(PathObject::INTERPOLATION_PROPERTY_KEY)->set(InterpolationMode::Bezier);
      m_scene.set_selection({m_current_path_object});
    }
    m_current_point = nullptr;
    m_last_point = nullptr;
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
  Scene& m_scene;
  std::unique_ptr<Macro> m_macro;
};

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
  if (SelectPointsBaseTool::mouse_press(pos, event, false)) {
    if (control_modifier) {
      m_path_builder->close_path();
    } else {
      m_path_builder->find_tie();
    }
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
