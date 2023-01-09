#include "commands/addremovepointscommand.h"
#include "commands/addremovepointscommandchangeset.h"
#include "logging.h"
#include "objects/pathobject.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathview.h"
#include "scene/scene.h"
#include "transform.h"
#include <memory>

namespace omm
{

AddRemovePointsCommand::AddRemovePointsCommand(const QString& label, AddRemovePointsCommandChangeSet changes,
                                               PathObject* const path_object)
  : Command(label)
  , m_change_set(std::make_unique<AddRemovePointsCommandChangeSet>(std::move(changes)))
  , m_path_object(path_object)
{
}

AddRemovePointsCommand::~AddRemovePointsCommand() = default;

void AddRemovePointsCommand::restore_bridges()
{
  m_change_set->swap();
  update();
}

void AddRemovePointsCommand::restore_edges()
{
  m_change_set->swap();
  update();
}

std::deque<Edge*> AddRemovePointsCommand::owned_edges() const
{
  std::deque<Edge*> new_edges;
  const auto& oe = m_change_set->owned_edges();
  new_edges.insert(new_edges.end(), oe.begin(), oe.end());
  return new_edges;
}

void AddRemovePointsCommand::update()
{
  if (m_path_object != nullptr) {
    m_path_object->update();
    m_path_object->scene()->update_tool();
  }
}

}  // namespace omm
