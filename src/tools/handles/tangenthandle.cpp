#include "tools/handles/tangenthandle.h"
#include "renderers/painter.h"
#include "tools/handles/abstractselecthandle.h"
#include "tools/handles/pointselecthandle.h"
#include "tools/tool.h"

namespace omm
{
TangentHandle::TangentHandle(Tool& tool, PointSelectHandle& master_handle, Tangent tangent)
    : Handle(tool), m_master_handle(master_handle), m_tangent(tangent)
{
}

double TangentHandle::draw_epsilon() const
{
  static constexpr auto draw_epsilon = 4.0;
  return draw_epsilon;
}

void TangentHandle::draw(QPainter& painter) const
{
  painter.setPen(ui_color("tangent"));
  painter.setBrush(ui_color("tangent fill"));
  const auto r = draw_epsilon();
  painter.drawEllipse(Tool::centered_rectangle(position, r));
}

bool TangentHandle::contains_global(const Vec2f& point) const
{
  const double dist = (point - position).euclidean_norm();
  return dist < interact_epsilon();
}

bool TangentHandle::mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e)
{
  Handle::mouse_move(delta, pos, e);
  if (status() == HandleStatus::Active) {
    m_master_handle.transform_tangent(delta, m_tangent);
    return true;
  } else {
    return false;
  }
}

}  // namespace omm
