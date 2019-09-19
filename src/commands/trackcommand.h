#include "commands/command.h"
#include <set>
#include <string>
#include <memory>

namespace omm
{

class AbstractPropertyOwner;
class Animator;
class Track;

class TracksCommand : public Command
{
protected:
  TracksCommand(const std::string& label, Animator& animator,
                const std::set<AbstractPropertyOwner*>& owners, const std::string& property_key);
  TracksCommand(const std::string& label, Animator& animator,
                std::set<std::unique_ptr<Track>> tracks);

protected:
  void remove();
  void insert();

private:
  Animator& m_animator;
  std::set<std::unique_ptr<Track>> m_tracks;
  const std::set<AbstractPropertyOwner*> m_owners;
  const std::string m_property_key;
};

class InsertTracksCommand : public TracksCommand
{
public:
  InsertTracksCommand(Animator& animator, std::set<std::unique_ptr<Track>> tracks);
  void undo() override { remove(); }
  void redo() override { insert(); }
};

class RemoveTracksCommand : public TracksCommand
{
public:
  RemoveTracksCommand(Animator& animator, const std::set<AbstractPropertyOwner*>& owners,
                      const std::string& property_key);
  void undo() override { insert(); }
  void redo() override { remove(); }
};

}  // namespace omm
