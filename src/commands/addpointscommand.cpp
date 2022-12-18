#include "commands/addpointscommand.h"

#include "commands/addremovepointscommandchangeset.h"
#include "commands/ownedlocatedpath.h"
#include "path/edge.h"
#include "path/pathview.h"

namespace
{

auto make_change_set_for_add(omm::OwnedLocatedPath points_to_add)
{
  omm::PathView path_view_to_remove{*points_to_add.path(), points_to_add.point_offset(), 0};
  return omm::AddRemovePointsCommandChangeSet{path_view_to_remove, points_to_add.create_edges(),
                                              points_to_add.single_point()};
}

}  // namespace

namespace omm
{

AddPointsCommand::AddPointsCommand(OwnedLocatedPath&& points_to_add, PathObject* const path_object)
  : AddRemovePointsCommand(static_label(), make_change_set_for_add(std::move(points_to_add)), path_object)
  , m_new_edges(owned_edges())  // owned_edges are new edges before calling redo.
{
}

void AddPointsCommand::undo()
{
  restore_bridges();
}

void AddPointsCommand::redo()
{
  restore_edges();
}

QString AddPointsCommand::static_label()
{
  return QObject::tr("AddPointsCommand");
}

std::deque<Edge*> AddPointsCommand::new_edges() const
{
  return m_new_edges;
}

}  // namespace omm
