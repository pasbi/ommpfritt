#pragma once

#include "commands/command.h"
#include <deque>
#include <memory>

namespace omm
{

class AddRemovePointsCommandChangeSet;
class Edge;
class PathObject;

class AddRemovePointsCommand : public Command
{
public:

  /**
   * @brief owned_edges the edges that this command owns.
   *  This changes when calling @code undo and @code redo, however, it is invariant when calling
   *  @code redo and @code undo subsequentially.
   */
  std::deque<Edge*> owned_edges() const;

protected:
  explicit AddRemovePointsCommand(const QString& label, AddRemovePointsCommandChangeSet changes,
                                  PathObject* path_object = nullptr);
  ~AddRemovePointsCommand() override;
  void restore_bridges();
  void restore_edges();

private:
  std::unique_ptr<AddRemovePointsCommandChangeSet> m_change_set;
  PathObject* m_path_object;
  void update();
};

}  // namespace omm
