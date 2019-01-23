#include "tools/brushselecttool.h"
#include "objects/path.h"
#include "scene/scene.h"
#include "properties/floatproperty.h"
#include <list>
#include <QMouseEvent>

namespace omm
{

const Style BrushSelectTool::m_style = ContourStyle(omm::Color(0.0, 0.0, 0.0));

BrushSelectTool::BrushSelectTool(Scene& scene)
  : Tool(scene)
  , position_variant(scene)
{
  this->template add_property<FloatProperty>(RADIUS_PROPERTY_KEY, 20.0)
    .set_label(QObject::tr("radius").toStdString())
    .set_category(QObject::tr("tool").toStdString());
}

QIcon BrushSelectTool::icon() const
{
  return QIcon();
}

bool BrushSelectTool::mouse_move( const arma::vec2& delta, const arma::vec2& pos,
                                  const QMouseEvent& event)
{
  if (m_mouse_down) {
    modify_selection(pos, event);
    m_mouse_pos = pos;
    return true;
  } else {
    return false;
  }
}

bool BrushSelectTool::mouse_press(const arma::vec2& pos, const QMouseEvent& event)
{
  if (event.modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
    // don't deselect
  } else {
    for (Object* object : scene.object_selection()) {
      auto* path = type_cast<Path*>(object);
      if (path != nullptr) { path->deselect_all_points(); }
    }
  }
  modify_selection(pos, event);
  m_mouse_down = true;
  m_mouse_pos = pos;
  return true;
}

void BrushSelectTool::mouse_release(const arma::vec2& pos, const QMouseEvent& event)
{
  m_mouse_down = false;
}

void BrushSelectTool
::modify_selection(const arma::vec2& pos, const QMouseEvent& event)
{
  const bool extend_selection = !(event.modifiers() & Qt::ControlModifier);
  const double radius = property(RADIUS_PROPERTY_KEY).value<double>();
  std::list<Point*> points;
  for (Object* object : scene.object_selection()) {
    Path* path = type_cast<Path*>(object);
    if (path) {
      for (Point* point : path->points()) {
        // we can't transform `pos` with path's inverse transformation because if it scales,
        // `radius` will be wrong.
        const auto gpos = path->global_transformation().apply_to_position(point->position);
        if (arma::norm(gpos - pos) < radius) {
          point->is_selected = extend_selection;
        }
      }
    }
  }
}

void BrushSelectTool::on_scene_changed()
{
  this->handles.clear();
  position_variant.make_handles(this->handles, *this);
}

void BrushSelectTool::on_selection_changed()
{
  on_scene_changed();
}

std::string BrushSelectTool::type() const { return TYPE; }

void BrushSelectTool::draw(AbstractRenderer& renderer) const
{
  Tool::draw(renderer);
  if (m_mouse_down) {
    const double radius = property(RADIUS_PROPERTY_KEY).value<double>();
    renderer.draw_circle(m_mouse_pos, radius, m_style);
  }
}

}  // namespace omm
