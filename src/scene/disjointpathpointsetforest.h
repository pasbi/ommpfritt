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
  void remove_dangling_points();
  void remove_if(const std::function<bool(const PathPoint* point)>& predicate);
  void replace(const std::map<PathPoint*, PathPoint*>& dict);

private:
  class ReferencePolisher;
  void serialize_impl(AbstractSerializer& serializer, const Pointer& root) const;
  void serialize(AbstractSerializer& serializer, const Joint& joint, const Pointer& root) const;
};

}  // namespace omm
