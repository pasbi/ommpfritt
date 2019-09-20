#include "aspects/animatablepropertyowner.h"

namespace omm
{

AnimatablePropertyOwner::AnimatablePropertyOwner(const AnimatablePropertyOwner &other)
  : AbstractPropertyOwner(other)
{

}

void AnimatablePropertyOwner::serialize(AbstractSerializer &serializer,
                                        const Serializable::Pointer &root) const
{
  AbstractPropertyOwner::serialize(serializer, root);
  const auto tracks_pointer = make_pointer(root, TRACKS_POINTER);
  serializer.start_array(m_tracks.size(), tracks_pointer);
  int i = 0;
  for (const auto& track : m_tracks) {
    track->serialize(serializer, make_pointer(tracks_pointer, i));
    i += 1;
  }
  serializer.end_array();

}

void AnimatablePropertyOwner::deserialize(AbstractDeserializer &deserializer,
                                          const Serializable::Pointer &root)
{
  AbstractPropertyOwner::deserialize(deserializer, root);

  const auto tracks_pointer = make_pointer(root, TRACKS_POINTER);
  const std::size_t n = deserializer.array_size(tracks_pointer);
  for (std::size_t i = 0; i < n; ++i) {
    auto track = std::make_unique<Track>();
    track->deserialize(deserializer, make_pointer(tracks_pointer, i));
    m_tracks.insert(std::move(track));
  }
}

void AnimatablePropertyOwner::apply_tracks(int frame)
{
  for (const auto& track : m_tracks) {
    track->apply(frame);
  }
}



}  // namespace omm
