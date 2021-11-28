#include "commands/subdividepathcommand.h"
#include "objects/pathobject.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"

namespace
{
using namespace omm;

auto compute_cuts(const Path& path)
{
  std::list<Geom::PathTime> cuts;
  const auto n = path.size();
  const auto points = path.points();
  for (std::size_t i = 0; i < n - 1; ++i) {
    if (points[i]->is_selected() && points[i + 1]->is_selected()) {
      static const double HALF_TIME = 0.5;
      cuts.emplace_back(i, HALF_TIME);
    }
  }
  return std::vector(cuts.begin(), cuts.end());
}

auto compute_cuts(const PathVector& path_vector)
{
  std::list<Geom::PathVectorTime> cuts;
  const auto paths = path_vector.paths();
  for (std::size_t i = 0; i < paths.size(); ++i) {
    for (auto&& cut : compute_cuts(*paths[i])) {
      cuts.emplace_back(i, cut);
    }
  }
  return std::vector(cuts.begin(), cuts.end());
}

}  // namespace

namespace omm
{
SubdividePathCommand::SubdividePathCommand(PathObject& path_object)
    : CutPathCommand(QObject::tr("Subdivide Path"), path_object, compute_cuts(path_object.geometry()))
{
}

}  // namespace omm
