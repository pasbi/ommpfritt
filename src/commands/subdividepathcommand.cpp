#include "commands/subdividepathcommand.h"
#include "objects/path.h"
#include "objects/path/pathpoint.h"
#include "objects/path/segment.h"

namespace
{
using namespace omm;

auto compute_cuts(const Segment& segment)
{
  std::list<Geom::PathTime> cuts;
  const auto n = segment.size();
  const auto points = segment.points();
  for (std::size_t i = 0; i < n - 1; ++i) {
    if (points[i]->is_selected() && points[i + 1]->is_selected()) {
      static const double HALF_TIME = 0.5;
      cuts.emplace_back(i, HALF_TIME);
    }
  }
  return std::vector(cuts.begin(), cuts.end());
}

auto compute_cuts(const Path& path)
{
  std::list<Geom::PathVectorTime> cuts;
  const auto segments = path.segments();
  for (std::size_t i = 0; i < segments.size(); ++i) {
    for (auto&& cut : compute_cuts(*segments[i])) {
      cuts.emplace_back(i, cut);
    }
  }
  return std::vector(cuts.begin(), cuts.end());
}

}  // namespace

namespace omm
{
SubdividePathCommand::SubdividePathCommand(Path& path)
    : CutPathCommand(Object::tr("Subdivide Path"), path, compute_cuts(path))
{
}

}  // namespace omm
