#include "commands/removepointscommand.h"
#include "commands/addremovepointscommandchangeset.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathview.h"

namespace
{

auto make_change_set_for_remove(const omm::PathView& path_view)
{
  std::deque<std::unique_ptr<omm::Edge>> edges;
  auto& path = path_view.path();

  if (path_view.begin() > 0 && path_view.end() < path.edges().size() + 1) {
    auto& left = *path.edges().at(path_view.begin() - 1);
    auto& right = *path.edges().at(path_view.end() - 1);
    edges.emplace_back(std::make_unique<omm::Edge>(left.a(), right.b(), &path));
  }
  return omm::AddRemovePointsCommandChangeSet{path_view, std::move(edges), {}};
}

}  // namespace

namespace omm
{

RemovePointsCommand::RemovePointsCommand(const PathView& points_to_remove, PathObject* const path_object)
  : AddRemovePointsCommand(static_label(), ::make_change_set_for_remove(points_to_remove), path_object)
{
}

void RemovePointsCommand::undo()
{
  restore_edges();
}

void RemovePointsCommand::redo()
{
  restore_bridges();
}

QString RemovePointsCommand::static_label()
{
  return QObject::tr("RemovePointsCommand");
}

}  // namespace omm
