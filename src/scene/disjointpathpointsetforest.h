#pragma once

#include "disjointset.h"
#include "aspects/serializable.h"
#include "serializers/deserializerworker.h"
#include "serializers/serializerworker.h"

namespace omm
{

class PathPoint;

class DisjointPathPointSetForest : public DisjointSetForest<PathPoint*>
{
public:
  using DisjointSetForest::DisjointSetForest;
  void deserialize(serialization::DeserializerWorker& worker);
  void serialize(serialization::SerializerWorker& worker) const;
  void remove_dangling_points();
  void remove_if(const std::function<bool(const PathPoint* point)>& predicate);
  void replace(const std::map<PathPoint*, PathPoint*>& dict);

private:
  class ReferencePolisher;
  void serialize_impl(serialization::SerializerWorker& worker) const;
  static void serialize(serialization::SerializerWorker& worker, const Joint& joint);
};

}  // namespace omm
