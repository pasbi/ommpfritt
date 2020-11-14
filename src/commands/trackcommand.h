#pragma once

#include "commands/command.h"
#include <map>
#include <memory>
#include <set>
#include <string>

namespace omm
{
class Track;
class Property;
class Animator;
class AbstractPropertyOwner;

class TracksCommand : public Command
{
protected:
  TracksCommand(Animator& animator,
                const QString& label,
                const std::map<AbstractPropertyOwner*, Property*>& properties);
  TracksCommand(Animator& animator,
                const QString& label,
                std::map<AbstractPropertyOwner*, std::unique_ptr<Track>> tracks);

protected:
  void remove();
  void insert();

private:
  Animator& m_animator;
  std::map<AbstractPropertyOwner*, std::unique_ptr<Track>> m_tracks;
  const std::map<AbstractPropertyOwner*, Property*> m_properties;
  const QString m_property_key;
};

class InsertTracksCommand : public TracksCommand
{
public:
  InsertTracksCommand(Animator& animator,
                      std::map<AbstractPropertyOwner*, std::unique_ptr<Track>> tracks);
  void undo() override
  {
    remove();
  }
  void redo() override
  {
    insert();
  }
};

class RemoveTracksCommand : public TracksCommand
{
public:
  RemoveTracksCommand(Animator& animator,
                      const std::map<AbstractPropertyOwner*, Property*>& properties);
  void undo() override
  {
    insert();
  }
  void redo() override
  {
    remove();
  }
};

}  // namespace omm
