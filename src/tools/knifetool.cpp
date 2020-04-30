#include "tools/knifetool.h"
#include <QCoreApplication>
#include "scene/scene.h"
#include "common.h"
#include "objects/path.h"
#include "commands/modifypointscommand.h"

namespace omm
{

KnifeTool::KnifeTool(Scene& scene) : SelectPointsBaseTool(scene) { }

bool KnifeTool::mouse_move(const Vec2f &delta, const Vec2f &pos, const QMouseEvent &e)
{
  m_mouse_move_pos = pos;
  if (SelectPointsBaseTool::mouse_move(delta, pos, e)) {
    return true;
  } else if (m_is_cutting) {
    m_points.clear();
    for (auto&& path : ::type_cast<Path*>(scene()->item_selection<Object>())) {
      auto ts = path->cut(m_mouse_press_pos, m_mouse_move_pos);
      const auto g = path->global_transformation(Space::Viewport);
      const auto ps = ::transform<Point>(ts, [&path, g](const double t) {
        return g.apply(path->evaluate(t));
      });
      m_points.insert(m_points.end(), ps.begin(), ps.end());
    }
  }
  return true;
}

bool KnifeTool::mouse_press(const Vec2f &pos, const QMouseEvent &event)
{
  m_mouse_press_pos = pos;
  if (SelectPointsBaseTool::mouse_press(pos, event)) {
    return true;
  } else {
    m_points.clear();
    m_is_cutting = true;
    return false;
  }
}


void KnifeTool::mouse_release(const Vec2f &pos, const QMouseEvent &event)
{
  if (m_is_cutting) {
    std::map<Path*, std::vector<Path::PointSequence>> sequencess;
    for (auto&& path : ::type_cast<Path*>(scene()->item_selection<Object>())) {
      const auto ts = path->cut(m_mouse_press_pos, m_mouse_move_pos);
      sequencess[path] = path->get_point_sequences(ts);
    }
    scene()->submit<AddPointsCommand>(sequencess);
    reset();
  }
  SelectPointsBaseTool::mouse_release(pos, event);
  m_is_cutting = false;
}

void KnifeTool::draw(Painter &renderer) const
{
  SelectPointsBaseTool::draw(renderer);
  if (m_is_cutting) {
    renderer.painter->setPen(ui_color(HandleStatus::Active, "Handle", "foreground"));
    renderer.painter->drawLine(m_mouse_press_pos.x, m_mouse_press_pos.y,
                               m_mouse_move_pos.x, m_mouse_move_pos.y);
    for (const Point& p : m_points) {
      renderer.painter->setPen(ui_color(HandleStatus::Active, "Handle", "marker"));
      const auto r = 3.0/2.0;
      renderer.painter->drawEllipse(p.position.x-r, p.position.y-r, 2*r, 2*r);
    }
  }
}

QString KnifeTool::name() const
{
  return QCoreApplication::translate("any-context", TYPE);
}

QString KnifeTool::type() const { return TYPE; }
void KnifeTool::cancel() { m_is_cutting = false; }

}  // namespace omm
