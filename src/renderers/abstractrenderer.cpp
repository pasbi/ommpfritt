#include "renderers/abstractrenderer.h"
#include "scene/scene.h"

namespace omm
{

AbstractRenderer::AbstractRenderer(const Region& region)
  : m_region(region)
{
}

void AbstractRenderer::render(const Scene& scene)
{
  scene.root().render(*this);
}

const AbstractRenderer::Region& AbstractRenderer::region() const
{
  return m_region;
}

AbstractRenderer::Region::Region(const arma::vec2& point)
  : AbstractRenderer::Region(point, point)
{
}

AbstractRenderer::Region::Region(const arma::vec2& top_left, const arma::vec2& top_right)
  : m_top_left(top_left), m_bottom_right(top_right)
{
  assert(m_bottom_right[0] >= m_top_left[0]);
  assert(m_bottom_right[1] >= m_top_left[1]);
}

AbstractRenderer::Region AbstractRenderer::Region::merge(const Region& other) const
{
  return Region( { std::min(m_top_left[0], other.m_top_left[0]),
                   std::min(m_top_left[1], other.m_top_left[1]) },
                 { std::max(m_bottom_right[0], other.m_bottom_right[0]),
                   std::max(m_bottom_right[1], other.m_bottom_right[1]) } );
}

AbstractRenderer::Region AbstractRenderer::Region::intersect(const Region& other) const
{
  const auto clamp = [](const auto& v, const auto& lo, const auto& hi) {
    return std::min(hi, std::max(lo, v));
  };

  return Region( { clamp(m_top_left[0], other.m_top_left[0], other.m_bottom_right[0]),
                   clamp(m_top_left[1], other.m_top_left[1], other.m_bottom_right[1]) },
                 { clamp(m_bottom_right[0], other.m_top_left[0], other.m_bottom_right[0]),
                   clamp(m_bottom_right[1], other.m_top_left[1], other.m_bottom_right[1]) } );
}

bool AbstractRenderer::Region::contains(const Region& other) const
{
  return other.contains(top_left())
      || other.contains(top_right())
      || other.contains(bottom_left())
      || other.contains(top_right());
}

bool AbstractRenderer::Region::contains(const arma::vec2& point) const
{
  return m_top_left[0] <= point[0] && m_bottom_right[0] <= point[0]
      && m_top_left[1] <= point[1] && m_bottom_right[1] <= point[1];
}

arma::vec2 AbstractRenderer::Region::top_left() const
{
  return m_top_left;
}

arma::vec2 AbstractRenderer::Region::top_right() const
{
  return { m_bottom_right[0], m_top_left[1] };
}

arma::vec2 AbstractRenderer::Region::bottom_left() const
{
  return { m_top_left[0], m_bottom_right[1] };
}

arma::vec2 AbstractRenderer::Region::bottom_right() const
{
  return m_bottom_right;
}

bool AbstractRenderer::Region::is_empty() const
{
  return arma::all(m_top_left == m_bottom_right);
}

}  // namespace omm
