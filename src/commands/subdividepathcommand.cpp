#include "commands/subdividepathcommand.h"
#include "objects/pathobject.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"

namespace
{
using namespace omm;


auto compute_cuts(const PathVector& path_vector)
{
  (void) path_vector;
  std::list<Geom::PathVectorTime> cuts;
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
