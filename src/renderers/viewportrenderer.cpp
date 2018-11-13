#include "renderers/viewportrenderer.h"
#include "renderers/style.h"

#include <QPainter>

namespace omm
{

ViewportRenderer::ViewportRenderer(QPainter& painter, const Region& region)
  : AbstractRenderer(region)
  , m_painter(painter)
{
}

void ViewportRenderer::draw_spline(const std::vector<Point>& points, const Style& style)
{

}

}  // namespace omm
