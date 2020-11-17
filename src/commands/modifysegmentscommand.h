#pragma once

#include "commands/command.h"
#include "objects/path.h"
#include <2geom/pathvector.h>

namespace omm
{
class Path;

class ModifySegmentsCommand : public Command
{
public:
  using Segments = std::vector<std::optional<Path::Segment>>;
  explicit ModifySegmentsCommand(const QString& label, Path& path, const Segments& segments);
  void undo() override;
  void redo() override;

private:
  Path& m_path;
  const Segments m_old_segments;
  const Segments m_new_segments;
  void apply(const Segments& segments);
};

}  // namespace omm
