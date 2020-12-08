#include "tools/knifetool.h"
#include "commands/cutpathcommand.h"
#include "commands/modifypointscommand.h"
#include "common.h"
#include "objects/path.h"
#include "scene/history/historymodel.h"
#include "scene/scene.h"
#include <2geom/line.h>
#include <2geom/path-intersection.h>
#include <QCoreApplication>

namespace
{
using namespace omm;

template<typename ResultFormat>
std::vector<ResultFormat>
compute_cut_points(const Geom::PathVector& path_vector, Vec2f start, Vec2f& end)
{
  static const auto line = [](const Vec2f& start, const Vec2f& end) {
    std::vector<Geom::CubicBezier> lines{std::vector<Geom::Point>{{start.x, start.y},
                                                                  {start.x, start.y},
                                                                  {end.x, end.y},
                                                                  {end.x, end.y}}};
    return Geom::PathVector(Geom::Path(lines.begin(), lines.end()));
  };
  const auto intersections = path_vector.intersect(line(start, end), 0.0);
  if constexpr (std::is_same_v<ResultFormat, Geom::PathVectorTime>) {
    return ::transform<Geom::PathVectorTime>(intersections,
                                             [](const auto& piv) { return piv.first; });
  } else {
    static_assert(std::is_same_v<ResultFormat, Vec2f>);
    return ::transform<Vec2f>(intersections, [](const auto& piv) {
      const Geom::Point p{piv};
      return Vec2f{p.x(), p.y()};
    });
  }
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
    for (auto&& path : ::type_cast<Path*>(scene()->item_selection<Object>())) {
      const auto path_vector
          = path->global_transformation(Space::Viewport).apply(path->geom_paths());
      const auto cut_points
          = compute_cut_points<Vec2f>(path_vector, m_mouse_press_pos, m_mouse_move_pos);
      m_points.insert(m_points.end(), cut_points.begin(), cut_points.end());
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
  Q_UNUSED(pos)
  Q_UNUSED(event)
  if (m_is_cutting) {
    if (const auto paths = ::type_cast<Path*>(scene()->item_selection<Object>()); !paths.empty()) {
      std::unique_ptr<Macro> macro;
      for (auto&& path : paths) {
        const auto path_vector
            = path->global_transformation(Space::Viewport).apply(path->geom_paths());
        const auto cut_points = compute_cut_points<Geom::PathVectorTime>(path_vector,
                                                                         m_mouse_press_pos,
                                                                         m_mouse_move_pos);
        if (!cut_points.empty()) {
          if (!macro) {
            macro = scene()->history().start_macro(QObject::tr("Cut Path"));
          }
          scene()->submit<CutPathCommand>(*path, cut_points);
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
    renderer.painter->drawLine(m_mouse_press_pos.x,
                               m_mouse_press_pos.y,
                               m_mouse_move_pos.x,
                               m_mouse_move_pos.y);
    for (const Point& p : m_points) {
      QPen pen;
      pen.setColor(Qt::white);
      ;
      pen.setWidthF(3);
      renderer.painter->setPen(pen);
      static constexpr double r = 6.0 / 2.0;
      renderer.painter->drawEllipse(centered_rectangle(p.position, r));
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
void KnifeTool::cancel()
{
  m_is_cutting = false;
}

SceneMode KnifeTool::scene_mode() const
{
  return SceneMode::Vertex;
}

void KnifeTool::reset()
{
  handles.clear();
  SelectPointsTool::make_handles(*this, false);
}

}  // namespace omm
