#include "tools/handles/bandhandle.h"
#include <armadillo>
#include "renderers/abstractrenderer.h"
#include "common.h"
#include <QDebug>

namespace omm
{

BandHandle::BandHandle(Tool& tool, bool transform_in_tool_space)
  : Handle(tool, transform_in_tool_space)
{
}

bool BandHandle::contains_global(const arma::vec2& point) const
{
  const arma::vec2 global_point = transformation().inverted().apply_to_position(point);
  const double x = global_point(0);
  const double y = global_point(1);
  return x+y < r + width/2.0 && x+y > r - width/2.0 && arma::all(global_point > stop);
}

void BandHandle::draw(omm::AbstractRenderer& renderer) const
{
  const auto path = std::vector { Point(arma::vec2{ stop, r - width/2.0 - stop }),
                                  Point(arma::vec2{ stop, r + width/2.0 - stop }),
                                  Point(arma::vec2{ r + width/2.0 - stop, stop }),
                                  Point(arma::vec2{ r - width/2.0 - stop, stop }) };
  renderer.draw_spline(path, current_style(), true);
}

}  // namespace omm
