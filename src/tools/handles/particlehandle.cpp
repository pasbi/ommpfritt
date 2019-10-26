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

void ParticleHandle::draw(QPainter &) const
{
}

}  // namespace omm
