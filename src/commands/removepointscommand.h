#pragma once

#include "commands/addremovepointscommand.h"
#include <QString>

namespace omm
{

class PathView;

class RemovePointsCommand : public AddRemovePointsCommand
{
public:
  explicit RemovePointsCommand(const PathView& points_to_remove, PathObject* path_object = nullptr);
  void undo() override;
  void redo() override;
  static QString static_label();
};

}  // namespace omm
