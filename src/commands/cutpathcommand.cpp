#include "commands/cutpathcommand.h"
#include "commands/modifypointscommand.h"
#include "commands/addremovepointscommand.h"
#include "objects/pathobject.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"

namespace
{

using namespace omm;


}  // namespace

namespace omm
{

CutPathCommand::CutPathCommand(PathObject& path_object, const std::vector<Geom::PathVectorTime>& cuts)
    : CutPathCommand(QObject::tr("CutPathCommand"), path_object, cuts)
{
}

CutPathCommand::CutPathCommand(const QString& label,
                               PathObject& path_object,
                               const std::vector<Geom::PathVectorTime>& cuts)
    : ComposeCommand(label)
{
  (void) label;
  (void) path_object;
  (void) cuts;
}

}  // namespace omm
