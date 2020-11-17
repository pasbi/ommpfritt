
#pragma once

#include "animation/track.h"
#include "commands/command.h"

namespace omm
{
class Property;

class SetInterpolationCommand : public Command
{
public:
  SetInterpolationCommand(const std::set<Property*>& properties,
                          Track::Interpolation interpolation);
  void undo()
  {
    swap();
  }
  void redo()
  {
    swap();
  }

private:
  void swap();
  std::map<Track*, Track::Interpolation> m_others;
};

}  // namespace omm
