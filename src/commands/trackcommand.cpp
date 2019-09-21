#include "commands/trackcommand.h" #include "animation/animator.h"
#include "common.h"
#include "animation/track.h"
#include "properties/property.h"

namespace omm
{

TracksCommand::TracksCommand(const std::string &label, const std::set<Property*> &properties)
  : Command(label)
  , m_tracks()
  , m_properties(properties)
{

}

TracksCommand::TracksCommand(const std::string &label, std::set<std::unique_ptr<Track>> tracks)
  : Command(label)
  , m_tracks(std::move(tracks))
  , m_properties(::transform<Property*>(m_tracks, [](const auto& track) { return &track->property(); }))
{
}

void TracksCommand::remove()
{
  assert(m_tracks.empty());
  for (Property* property : m_properties) {
    m_tracks.insert(property->extract_track());
  }
}

void TracksCommand::insert()
{
  while (!m_tracks.empty()) {
    std::unique_ptr<Track> track = std::move(m_tracks.extract(m_tracks.begin()).value());
    track->property().set_track(std::move(track));
  }
  m_tracks.clear();
}

InsertTracksCommand::InsertTracksCommand(std::set<std::unique_ptr<Track>> tracks)
  : TracksCommand(QObject::tr("Create Tracks").toStdString(), std::move(tracks))
{
}

RemoveTracksCommand::RemoveTracksCommand(const std::set<Property*> &properties)
  : TracksCommand(QObject::tr("Remove Tracks").toStdString(), properties)
{
}


}  // namespace omm
