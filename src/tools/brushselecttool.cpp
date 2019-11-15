#include "tools/brushselecttool.h"
#include "objects/path.h"
#include "scene/scene.h"
#include "properties/floatproperty.h"
#include "tools/selectpointstool.h"
#include <list>
#include <QMouseEvent>
#include "scene/messagebox.h"

namespace omm
{

BrushSelectTool::BrushSelectTool(Scene& scene) : SelectPointsBaseTool(scene)
{
  create_property<FloatProperty>(RADIUS_PROPERTY_KEY, 20.0)
    .set_label(QObject::tr("radius"))
    .set_category(QObject::tr("tool"))
    .set_animatable(false);
}

bool BrushSelectTool::mouse_move( const Vec2f& delta, const Vec2f& pos,
                                  const QMouseEvent& event)
{
  if (SelectPointsBaseTool::mouse_move(delta, pos, event)) {
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

bool BrushSelectTool::mouse_press(const Vec2f& pos, const QMouseEvent& event)
{
  if (event.modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
    // don't deselect
  } else {
    if (SelectPointsBaseTool::mouse_press(pos, event)) {
      return true;
    }
    for (Object* object : scene()->item_selection<Object>()) {
      auto* path = type_cast<Path*>(object);
      if (path != nullptr) {
        path->deselect_all_points();
      }
    }
  }
  modify_selection(pos, event);
  m_mouse_down = true;
  m_mouse_pos = pos;
  return true;
}

void BrushSelectTool::mouse_release(const Vec2f& pos, const QMouseEvent& event)
{
  SelectPointsBaseTool::mouse_release(pos, event);
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
  bool is_noop = true;
  for (Object* object : scene()->item_selection<Object>()) {
    Path* path = type_cast<Path*>(object);
    if (path) {
      for (Point* point : path->points_ref()) {
        // we can't transform `pos` with path's inverse transformation because if it scales,
        // `radius` will be wrong.
        const auto gt = path->global_transformation(Space::Viewport);
        const auto gpos = gt.apply_to_position(point->position);
        if ((gpos - pos).euclidean_norm() < radius) {
          if (point->is_selected != extend_selection) {
            is_noop = false;
            point->is_selected = extend_selection;
          }
        }
      }
    }
  }
  if (!is_noop) {
    Q_EMIT scene()->message_box().point_selection_changed();
  }
}

void BrushSelectTool::reset()
{
  handles.clear();
  SelectPointsTool::make_handles(*this, false);
}

QString BrushSelectTool::type() const { return TYPE; }

void BrushSelectTool::draw(Painter &renderer) const
{
  SelectPointsBaseTool::draw(renderer);
  if (m_mouse_down) {
    const double r = property(RADIUS_PROPERTY_KEY)->value<double>();
    renderer.painter->setPen(ui_color(HandleStatus::Active, "Handle", "foreground"));
    renderer.painter->setBrush(ui_color(HandleStatus::Active, "Handle", "background"));
    renderer.painter->drawEllipse(m_mouse_pos.x - r, m_mouse_pos.y - r, 2*r, 2*r);
  }
}

QString BrushSelectTool::name() const
{
  return QCoreApplication::translate("any-context", TYPE);
}

}  // namespace omm
