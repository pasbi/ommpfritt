#include "commands/trackcommand.h"
#include "animation/animator.h"
#include "common.h"
#include "animation/track.h"

namespace omm
{

TracksCommand::TracksCommand(const std::string &label, Animator &animator,
                             const std::set<AbstractPropertyOwner *> &owners,
                             const std::string &property_key)
  : Command(label)
  , m_animator(animator)
  , m_tracks()
  , m_owners(owners)
  , m_property_key(property_key)
{

}

TracksCommand::TracksCommand(const std::string &label, Animator &animator,
                             std::set<std::unique_ptr<Track>> tracks)
  : Command(label)
  , m_animator(animator)
  , m_tracks(std::move(tracks))
  , m_owners(::transform<AbstractPropertyOwner*>(m_tracks, [](const auto& track) { return track->owner(); }))
  , m_property_key((*m_tracks.begin())->property_key())
{
}

void TracksCommand::remove()
{
  assert(m_tracks.empty());
  for (AbstractPropertyOwner* owner : m_owners) {
    m_tracks.insert(m_animator.extract_track(*owner, m_property_key));
  }
}

void TracksCommand::insert()
{
  while (!m_tracks.empty()) {
    m_animator.insert_track(std::move(m_tracks.extract(m_tracks.begin()).value()));
  }
  m_tracks.clear();
}

InsertTracksCommand::InsertTracksCommand(Animator &animator,
                                         std::set<std::unique_ptr<Track> > tracks)
  : TracksCommand(QObject::tr("Create Tracks").toStdString(), animator, std::move(tracks))
{
}

RemoveTracksCommand::RemoveTracksCommand(Animator &animator,
                                         const std::set<AbstractPropertyOwner *> &owners,
                                         const std::string &property_key)
  : TracksCommand(QObject::tr("Remove Tracks").toStdString(), animator, owners, property_key)
{
}


}  // namespace omm
