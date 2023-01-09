#include "commands/subdividepathcommand.h"
#include "commands/cutpathcommand.h"
#include "objects/pathobject.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"

namespace
{
using namespace omm;


auto compute_cuts(const PathVector& path_vector)
{
  std::list<Geom::PathVectorTime> cuts;
  const auto paths = path_vector.paths();
  for (std::size_t path_index = 0; path_index < paths.size(); ++path_index) {
    const auto edges = paths.at(path_index)->edges();
    for (std::size_t edge_index = 0; edge_index < edges.size(); ++edge_index) {
      const auto& edge = *edges.at(edge_index);
      if (edge.a()->is_selected() && edge.b()->is_selected()) {
        cuts.emplace_back(path_index, edge_index, 0.5);
      }
    }
  }
  return std::vector(cuts.begin(), cuts.end());
}

}  // namespace

namespace omm
{

SubdividePathCommand::SubdividePathCommand(PathObject& path_object)
  : CutPathCommand(QObject::tr("Subdivide Path"), path_object, compute_cuts(path_object.path_vector()))
{
  for (auto* const point : new_points()) {
    point->set_selected(true);
  }
}

}  // namespace omm
