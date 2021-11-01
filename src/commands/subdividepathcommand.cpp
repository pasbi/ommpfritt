#include "commands/subdividepathcommand.h"
#include "objects/path.h"

namespace
{
using namespace omm;

auto compute_cuts(const Segment& segment, bool is_closed)
{
  std::list<Geom::PathTime> cuts;
  const auto off = is_closed ? 0 : 1;
  const auto n = segment.size();
  for (std::size_t i = 0; i < n - off; ++i) {
    if (segment[i].is_selected && segment[(i + 1) % n].is_selected) {
      static const double HALF_TIME = 0.5;
      cuts.emplace_back(i, HALF_TIME);
    }
  }
  return std::vector(cuts.begin(), cuts.end());
}

auto compute_cuts(const Path& path)
{
  std::list<Geom::PathVectorTime> cuts;
  const bool is_closed = path.is_closed();
  for (std::size_t i = 0; i < path.segments.size(); ++i) {
    for (auto&& cut : compute_cuts(path.segments[i], is_closed)) {
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
