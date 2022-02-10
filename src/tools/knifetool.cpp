#include "tools/knifetool.h"
#include "commands/cutpathcommand.h"
#include "commands/modifypointscommand.h"
#include "common.h"
#include "geometry/point.h"
#include "objects/pathobject.h"
#include "path/pathvector.h"
#include "path/lib2geomadapter.h"
#include "preferences/uicolors.h"
#include "renderers/painter.h"
#include "scene/history/historymodel.h"
#include "scene/history/macro.h"
#include "scene/scene.h"
#include <2geom/line.h>
#include <2geom/path-intersection.h>
#include <QCoreApplication>
#include <QMouseEvent>

namespace
{
using namespace omm;

std::vector<Geom::Intersection<Geom::PathVectorTime>>
compute_cut_points(const Geom::PathVector& path_vector, const Vec2f& start, const Vec2f& end)
{
  static const auto line = [](const Vec2f& start, const Vec2f& end) {
    std::vector<Geom::CubicBezier> lines{std::vector<Geom::Point>{{start.x, start.y},
                                                                  {start.x, start.y},
                                                                  {end.x, end.y},
                                                                  {end.x, end.y}}};
    return Geom::PathVector(Geom::Path(lines.begin(), lines.end()));
  };
  return path_vector.intersect(line(start, end), 0.0);
}

Geom::PathVector get_global_path_vector(const PathObject& po)
{
  const auto path_vector = omm_to_geom(po.path_vector());
  const auto transformation = po.global_transformation(Space::Viewport);
  return transformation.apply(path_vector);
}

}  // namespace

namespace omm
{

KnifeTool::KnifeTool(Scene& scene) : SelectPointsBaseTool(scene)
{
}

bool KnifeTool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  m_mouse_move_pos = pos;
  if (SelectPointsBaseTool::mouse_move(delta, pos, e)) {
    return true;
  } else if (m_is_cutting) {
    m_points.clear();
    for (auto&& path_object : ::type_casts<PathObject*>(scene()->item_selection<Object>())) {
      const auto global_path_vector = get_global_path_vector(*path_object);
      const auto intersections = compute_cut_points(global_path_vector, m_mouse_press_pos, m_mouse_move_pos);
      const auto points = util::transform(intersections, [](const auto& t) {
        const Geom::Point p{t};
        return Vec2f{p.x(), p.y()};
      });
      m_points.insert(m_points.end(), std::move_iterator{points.begin()}, std::move_iterator{points.end()});
    }
    return true;
  } else {
    return false;
  }
}

bool KnifeTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  reset();
  m_mouse_press_pos = pos;
  if (SelectPointsBaseTool::mouse_press(pos, event)) {
    return true;
  } else if (event.modifiers() == Qt::NoModifier) {
    m_points.clear();
    m_is_cutting = true;
    return true;
  } else {
    return false;
  }
}

void KnifeTool::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  if (m_is_cutting) {
    if (const auto path_objects = ::type_casts<PathObject*>(scene()->item_selection<Object>()); !path_objects.empty()) {
      std::unique_ptr<Macro> macro;
      for (auto&& path_object : path_objects) {
        const auto global_path_vector = get_global_path_vector(*path_object);
        const auto intersections = compute_cut_points(global_path_vector, m_mouse_press_pos, m_mouse_move_pos);
        const auto ts = util::transform(intersections, [](const auto& piv) { return piv.first; });
        if (!ts.empty()) {
          if (!macro) {
            macro = scene()->history().start_macro(QObject::tr("Cut Path"));
          }
          scene()->submit<CutPathCommand>(*path_object, ts);
        }
      }
    }
    reset();
  }
  SelectPointsBaseTool::mouse_release(pos, event);
  m_is_cutting = false;
}

void KnifeTool::draw(Painter& renderer) const
{
  SelectPointsBaseTool::draw(renderer);
  if (m_is_cutting) {
    renderer.painter->setPen(ui_color(HandleStatus::Active, "Handle", "foreground"));
    renderer.painter->drawLine(static_cast<int>(m_mouse_press_pos.x),
                               static_cast<int>(m_mouse_press_pos.y),
                               static_cast<int>(m_mouse_move_pos.x),
                               static_cast<int>(m_mouse_move_pos.y));
    for (const Point& p : m_points) {
      QPen pen;
      pen.setColor(Qt::white);
      pen.setWidthF(3);
      renderer.painter->setPen(pen);
      static constexpr double r = 6.0 / 2.0;
      renderer.painter->drawEllipse(centered_rectangle(p.position(), r));
    }
  }
}

QString KnifeTool::name() const
{
  return QCoreApplication::translate("any-context", TYPE);
}

QString KnifeTool::type() const
{
  return TYPE;
}

bool KnifeTool::cancel()
{
  if (m_is_cutting) {
    m_is_cutting = false;
    return true;
  } else {
    return false;
  }
}

SceneMode KnifeTool::scene_mode() const
{
  return SceneMode::Vertex;
}

}  // namespace omm
