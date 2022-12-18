#include "commands/modifypointscommand.h"
#include "common.h"
#include "scene/scene.h"
#include "objects/pathobject.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/pathview.h"


namespace omm
{

ModifyPointsCommand ::ModifyPointsCommand(const std::map<PathPoint*, Point>& points)
    : Command(QObject::tr("ModifyPointsCommand")), m_data(points)
{
  assert(!points.empty());
}

void ModifyPointsCommand::undo()
{
  exchange();
}

void ModifyPointsCommand::redo()
{
  exchange();
}

int ModifyPointsCommand::id() const
{
  return Command::MODIFY_TANGENTS_COMMAND_ID;
}

void ModifyPointsCommand::exchange()
{
  std::set<const PathVector*> path_vectors;
  for (auto& [ptr, point] : m_data) {
    const auto geometry = ptr->geometry();
    ptr->set_geometry(point);
    point = geometry;
    path_vectors.insert(ptr->path_vector());
  }
  for (const auto* const path_vector : path_vectors) {
    path_vector->path_object()->update();
  }
}

bool ModifyPointsCommand::mergeWith(const QUndoCommand* command)
{
  // merging happens automatically!
  const auto& mtc = dynamic_cast<const ModifyPointsCommand&>(*command);
  return ::get_keys(m_data) == ::get_keys(mtc.m_data);
}

bool ModifyPointsCommand::is_noop() const
{
  return std::all_of(m_data.begin(), m_data.end(), [](const auto& arg) {
    const auto& [ptr, new_value] = arg;
    return ptr->geometry() == new_value;
  });
}

}  // namespace omm
