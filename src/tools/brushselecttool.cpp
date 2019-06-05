#include "tools/brushselecttool.h"
#include "objects/path.h"
#include "scene/scene.h"
#include "properties/floatproperty.h"
#include "tools/selectpointstool.h"
#include <list>
#include <QMouseEvent>

namespace omm
{

const Style BrushSelectTool::m_style = ContourStyle(omm::Color(0.0, 0.0, 0.0));

BrushSelectTool::BrushSelectTool(Scene& scene) : SelectPointsTool(scene)
{
  this->template add_property<FloatProperty>(RADIUS_PROPERTY_KEY, 20.0)
    .set_label(QObject::tr("radius").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

bool BrushSelectTool::mouse_move( const Vec2f& delta, const Vec2f& pos,
                                  const QMouseEvent& event)
{
  if (SelectPointsTool::mouse_move(delta, pos, event)) {
    return true;
  }

  Q_UNUSED(delta);
  if (m_mouse_down) {
    modify_selection(pos, event);
    m_mouse_pos = pos;
    return true;
  } else {
    return false;
  }
}

bool BrushSelectTool::mouse_press(const Vec2f& pos, const QMouseEvent& event, bool force)
{
  if (SelectPointsTool::mouse_press(pos, event, force)) {
    return true;
  }

  Q_UNUSED(force);
  if (event.modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
    // don't deselect
  } else {
    for (Object* object : scene.item_selection<Object>()) {
      auto* path = type_cast<Path*>(object);
      if (path != nullptr) { path->deselect_all_points(); }
    }
  }
  modify_selection(pos, event);
  m_mouse_down = true;
  m_mouse_pos = pos;
  return true;
}

void BrushSelectTool::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  SelectPointsTool::mouse_release(pos, event);
  Q_UNUSED(pos);
  Q_UNUSED(event);
  m_mouse_down = false;
}

void BrushSelectTool
::modify_selection(const Vec2f& pos, const QMouseEvent& event)
{
  const bool extend_selection = !(event.modifiers() & Qt::ControlModifier);
  const double radius = property(RADIUS_PROPERTY_KEY)->value<double>();
  std::list<Point*> points;
  for (Object* object : scene.item_selection<Object>()) {
    Path* path = type_cast<Path*>(object);
    if (path) {
      for (Point* point : path->points_ref()) {
        // we can't transform `pos` with path's inverse transformation because if it scales,
        // `radius` will be wrong.
        const auto gpos = path->global_transformation().apply_to_position(point->position);
        if ((gpos - pos).euclidean_norm() < radius) {
          point->is_selected = extend_selection;
        }
      }
    }
  }
}

void BrushSelectTool::on_scene_changed()
{
  handles.clear();
  SelectPointsTool::make_handles(*this, false);
}

void BrushSelectTool::on_selection_changed()
{
  on_scene_changed();
}

std::string BrushSelectTool::type() const { return TYPE; }

void BrushSelectTool::draw(Painter &renderer) const
{
  Tool::draw(renderer);
  if (m_mouse_down) {
    const double r = property(RADIUS_PROPERTY_KEY)->value<double>();
    renderer.set_style(m_style);
    renderer.painter->drawEllipse(m_mouse_pos.x - r, m_mouse_pos.y - r, 2*r, 2*r);
  }
}

std::string BrushSelectTool::name() const
{
  return QCoreApplication::translate("any-context", TYPE).toStdString();
}

}  // namespace omm
