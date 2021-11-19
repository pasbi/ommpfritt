#pragma once

#include "disjointset.h"
#include "aspects/serializable.h"
#include "serializers/abstractserializer.h"

namespace omm
{

class PathPoint;

class DisjointPathPointSetForest : public DisjointSetForest<PathPoint*>, public Serializable, public ReferencePolisher
{
public:
  using DisjointSetForest::DisjointSetForest;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;

protected:
  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override;

private:
  struct PathPointId
  {
    constexpr explicit PathPointId(const std::size_t path_id, const std::size_t point_index)
      : path_id(path_id)
      , point_index(point_index)
    {
    }
    std::size_t path_id;
    std::size_t point_index;
  };
  std::deque<std::list<PathPointId>> m_joined_point_indices;
  friend class DisjointPathPointSetForestReferencePolisher;
};

}  // namespace omm
