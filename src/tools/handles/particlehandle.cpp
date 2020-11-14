#include "tools/handles/particlehandle.h"
#include <QPainter>

namespace omm
{
ParticleHandle::ParticleHandle(Tool& tool) : Handle(tool)
{
}

bool ParticleHandle::contains_global(const Vec2f& point) const
{
  const auto gpos = tool.transformation().apply_to_position(position);
  const double dist = (point - gpos).euclidean_norm();
  return dist < interact_epsilon();
}

void ParticleHandle::draw(QPainter& painter) const
{
  const auto pos = tool.transformation().null();
  const auto r = draw_epsilon();

  const QRectF rect{pos.x - r, pos.y - r, 2 * r, 2 * r};
  painter.fillRect(rect, ui_color(HandleStatus::Active, "particle"));
}

}  // namespace omm
