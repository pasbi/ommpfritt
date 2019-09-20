#pragma once

#include "abstractpropertyowner.h"
#include <set>
#include <memory>
#include "animation/track.h"

namespace omm
{

class AnimatablePropertyOwner : public AbstractPropertyOwner
{
protected:
  using AbstractPropertyOwner::AbstractPropertyOwner;
  AnimatablePropertyOwner(const AnimatablePropertyOwner& other);

public:
  void serialize(AbstractSerializer &serializer, const Pointer &root) const override;
  void deserialize(AbstractDeserializer &deserializer, const Pointer &root) override;
  static constexpr auto TRACKS_POINTER = "tracks";
  void apply_tracks(int frame);

private:
  std::set<std::unique_ptr<Track>> m_tracks;
};

}  // namespace
