#include "commands/command.h"
#include <set>
#include <string>
#include <memory>

namespace omm
{

class Track;
class Property;

class TracksCommand : public Command
{
protected:
  TracksCommand(const std::string& label, const std::set<Property*>& properties);
  TracksCommand(const std::string& label, std::set<std::unique_ptr<Track>> tracks);

protected:
  void remove();
  void insert();

private:
  std::set<std::unique_ptr<Track>> m_tracks;
  const std::set<Property*> m_properties;
  const std::string m_property_key;
};

class InsertTracksCommand : public TracksCommand
{
public:
  InsertTracksCommand(std::set<std::unique_ptr<Track>> tracks);
  void undo() override { remove(); }
  void redo() override { insert(); }
};

class RemoveTracksCommand : public TracksCommand
{
public:
  RemoveTracksCommand(const std::set<Property*>& properties);
  void undo() override { insert(); }
  void redo() override { remove(); }
};

}  // namespace omm
