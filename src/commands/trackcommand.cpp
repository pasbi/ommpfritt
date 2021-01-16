#include "commands/trackcommand.h"
#include "animation/animator.h"
#include "animation/track.h"
#include "common.h"
#include "properties/property.h"

namespace
{
std::map<omm::AbstractPropertyOwner*, omm::Property*>
collect(const std::map<omm::AbstractPropertyOwner*, std::unique_ptr<omm::Track>>& map)
{
  std::map<omm::AbstractPropertyOwner*, omm::Property*> collection;
  for (auto&& [owner, track] : map) {
    collection.insert(std::pair(owner, &track->property()));
  }
  return collection;
}

}  // namespace

namespace omm
{
TracksCommand::TracksCommand(Animator& animator,
                             const QString& label,
                             const std::map<AbstractPropertyOwner*, Property*>& properties)
    : Command(label), m_animator(animator), m_properties(properties)
{
}

TracksCommand::TracksCommand(Animator& animator,
                             const QString& label,
                             std::map<AbstractPropertyOwner*, std::unique_ptr<Track>> tracks)
    : Command(label), m_animator(animator), m_tracks(std::move(tracks)),
      m_properties(::collect(m_tracks))
{
}

void TracksCommand::remove()
{
  assert(m_tracks.empty());
  for (auto&& [owner, property] : m_properties) {
    if (auto track = m_animator.extract_track(*property); track) {
      m_tracks.insert(std::pair(owner, std::move(track)));
    }
  }
}

void TracksCommand::insert()
{
  while (!m_tracks.empty()) {
    auto node = m_tracks.extract(m_tracks.begin());
    m_animator.insert_track(*node.key(), std::move(node.mapped()));
  }
  m_tracks.clear();
}

InsertTracksCommand ::InsertTracksCommand(
    Animator& animator,
    std::map<AbstractPropertyOwner*, std::unique_ptr<Track>> tracks)
    : TracksCommand(animator, QObject::tr("Create Tracks"), std::move(tracks))
{
}

RemoveTracksCommand ::RemoveTracksCommand(
    Animator& animator,
    const std::map<AbstractPropertyOwner*, Property*>& properties)
    : TracksCommand(animator, QObject::tr("Remove Tracks"), properties)
{
}

}  // namespace omm
