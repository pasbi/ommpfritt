#pragma once

#include "disjointset.h"
#include "aspects/serializable.h"
#include "serializers/abstractserializer.h"

namespace omm
{

class PathPoint;

class DisjointPathPointSetForest : public DisjointSetForest<PathPoint*>, public Serializable
{
public:
  using DisjointSetForest::DisjointSetForest;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;

private:
  class ReferencePolisher;
};

}  // namespace omm
