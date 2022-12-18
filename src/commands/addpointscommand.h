#pragma once

#include "commands/addremovepointscommand.h"

namespace omm
{

class OwnedLocatedPath;

class AddPointsCommand : public AddRemovePointsCommand
{
public:
  explicit AddPointsCommand(OwnedLocatedPath&& points_to_add, PathObject* path_object = nullptr);
  void undo() override;
  void redo() override;
  static QString static_label();
  /**
   * @brief new_edges the edges that are created when calling @code redo.
   *  After calling @code undo, this is the same as @code owned_edges.
   */
  std::deque<Edge*> new_edges() const;

private:
  const std::deque<Edge*> m_new_edges;
};

}  // namespace omm
