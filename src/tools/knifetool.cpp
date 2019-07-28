#include "tools/knifetool.h"
#include <QCoreApplication>
#include "scene/scene.h"
#include "common.h"
#include "objects/path.h"
#include "commands/modifypointscommand.h"

namespace
{

const omm::SolidStyle marker_style(omm::Color(0, 0, 1));

}  // namespace

namespace omm
{

const Style KnifeTool::m_line_style = ContourStyle(Color(0.0, 0.0, 0.0, 1.0), 1);

KnifeTool::KnifeTool(Scene& scene) : SelectPointsBaseTool(scene) { }

bool KnifeTool::mouse_move(const Vec2f &delta, const Vec2f &pos, const QMouseEvent &e)
{
  m_mouse_move_pos = pos;
  if (SelectPointsBaseTool::mouse_move(delta, pos, e)) {
    return true;
  } else if (m_is_cutting) {
    m_points.clear();
    for (auto&& path : ::type_cast<Path*>(scene.item_selection<Object>())) {
      auto ts = path->cut(m_mouse_press_pos, m_mouse_move_pos);
      const auto g = path->global_transformation(false);
      const auto ps = ::transform<Point>(ts, [&path, g](const double t) {
        return g.apply(path->evaluate(t));
      });
      m_points.insert(m_points.end(), ps.begin(), ps.end());
    }
  }
  return true;
}

bool KnifeTool::mouse_press(const Vec2f &pos, const QMouseEvent &event, bool force)
{
  m_mouse_press_pos = pos;
  if (SelectPointsBaseTool::mouse_press(pos, event, force)) {
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
    for (auto&& path : ::type_cast<Path*>(scene.item_selection<Object>())) {
      const auto ts = path->cut(m_mouse_press_pos, m_mouse_move_pos);
      sequencess[path] = path->get_point_sequences(ts);
    }
    scene.submit<AddPointsCommand>(sequencess);
    on_selection_changed();
  }
  SelectPointsBaseTool::mouse_release(pos, event);
  m_is_cutting = false;
}

void KnifeTool::draw(Painter &renderer) const
{
  SelectPointsBaseTool::draw(renderer);
  if (m_is_cutting) {
    const auto line = std::vector { Point(m_mouse_press_pos), Point(m_mouse_move_pos) };

    renderer.set_style(m_line_style);
    renderer.painter->drawLine(m_mouse_press_pos.x, m_mouse_press_pos.y,
                               m_mouse_move_pos.x, m_mouse_move_pos.y);
    for (const Point& p : m_points) {
      renderer.set_style(marker_style);
      const auto r = 3.0/2.0;
      renderer.painter->drawEllipse(p.position.x-r, p.position.y-r, 2*r, 2*r);
    }
  }
}

std::string KnifeTool::name() const
{
  return QCoreApplication::translate("any-context", TYPE).toStdString();
}

std::string KnifeTool::type() const { return TYPE; }
void KnifeTool::cancel() { m_is_cutting = false; }

}  // namespace omm
