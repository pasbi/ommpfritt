#include "tools/brushselecttool.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "preferences/uicolors.h"
#include "properties/floatproperty.h"
#include "renderers/painter.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tools/selectpointstool.h"
#include <QMouseEvent>
#include <list>

namespace omm
{
BrushSelectTool::BrushSelectTool(Scene& scene) : SelectPointsBaseTool(scene)
{
  static constexpr double DEFAULT_RADIUS = 20.0;
  create_property<FloatProperty>(RADIUS_PROPERTY_KEY, DEFAULT_RADIUS)
      .set_label(QObject::tr("radius"))
      .set_category(QObject::tr("tool"))
      .set_animatable(false);
}

bool BrushSelectTool::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& event)
{
  if (SelectPointsBaseTool::mouse_move(delta, pos, event)) {
    return true;
  }
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
  if ((event.modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) != 0u) {
    // don't deselect
  } else {
    if (SelectPointsBaseTool::mouse_press(pos, event)) {
      return true;
    }
    for (Object* object : scene()->item_selection<Object>()) {
      if (auto* path_object = type_cast<PathObject*>(object); path_object != nullptr) {
        for (auto* point : path_object->geometry().points()) {
          point->set_selected(false);
        }
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

void BrushSelectTool ::modify_selection(const Vec2f& pos, const QMouseEvent& event)
{
  const bool extend_selection = !(event.modifiers() & Qt::ControlModifier);
  const auto radius = property(RADIUS_PROPERTY_KEY)->value<double>();
  std::list<Point*> points;
  bool is_noop = true;
  for (Object* object : scene()->item_selection<Object>()) {
    auto* path_object = type_cast<PathObject*>(object);
    if (path_object != nullptr) {
      for (auto* point : path_object->geometry().points()) {
        // we can't transform `pos` with path's inverse transformation because if it scales,
        // `radius` will be wrong.
        const auto gt = path_object->global_transformation(Space::Viewport);
        const auto gpos = gt.apply_to_position(point->geometry().position());
        if ((gpos - pos).euclidean_norm() < radius) {
          if (point->is_selected() != extend_selection) {
            is_noop = false;
            point->set_selected(extend_selection);
          }
        }
      }
    }
  }
  if (!is_noop) {
    Q_EMIT scene()->mail_box().point_selection_changed();
  }
}

QString BrushSelectTool::type() const
{
  return TYPE;
}

void BrushSelectTool::draw(Painter& renderer) const
{
  SelectPointsBaseTool::draw(renderer);
  if (m_mouse_down) {
    const auto r = property(RADIUS_PROPERTY_KEY)->value<double>();
    renderer.painter->setPen(ui_color(HandleStatus::Active, "Handle", "foreground"));
    renderer.painter->setBrush(ui_color(HandleStatus::Active, "Handle", "background"));
    renderer.painter->drawEllipse(centered_rectangle(m_mouse_pos, r));
  }
}

QString BrushSelectTool::name() const
{
  return QCoreApplication::translate("any-context", TYPE);
}

}  // namespace omm
