#include "scene/disjointpathpointsetforest.h"
#include "serializers/abstractserializer.h"
#include "objects/pathpoint.h"
#include "objects/path.h"

static constexpr auto FOREST_POINTER = "forest";
static constexpr auto PATH_ID_POINTER = "path-id";
static constexpr auto INDEX_POINTER = "index";

namespace
{

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

}  // namespace

namespace omm
{

class DisjointPathPointSetForest::ReferencePolisher : public omm::ReferencePolisher
{
public:
  explicit ReferencePolisher(const std::deque<std::list<PathPointId>>& joined_point_indices,
                             DisjointPathPointSetForest& ref)
    : m_ref(ref)
    , m_joined_point_indices(joined_point_indices)
  {
  }

private:
  omm::DisjointPathPointSetForest& m_ref;
  std::deque<std::list<PathPointId>> m_joined_point_indices;

  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override
  {
    m_ref.m_forest.clear();
    for (const auto& set : m_joined_point_indices) {
      auto& forest_set = m_ref.m_forest.emplace_back();
      for (const auto& [path_id, point_index] : set) {
        auto* path = dynamic_cast<Path*>(map.at(path_id));
        auto& path_point = path->point_at_index(point_index);
        forest_set.insert(&path_point);
      }
    }
  }
};

void DisjointPathPointSetForest::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  std::deque<std::list<PathPointId>> joined_point_indices;
  const auto forest_ptr = make_pointer(root, FOREST_POINTER);
  const auto forest_size = deserializer.array_size(forest_ptr);
  for (std::size_t i = 0; i < forest_size; ++i) {
    const auto set_ptr = make_pointer(forest_ptr, i);
    const auto set_size = deserializer.array_size(set_ptr);
    std::list<PathPointId> point_set;
    for (std::size_t j = 0; j < set_size; ++j) {
      const auto ptr = make_pointer(set_ptr, j);
      const auto point_index = deserializer.get_size_t(make_pointer(ptr, INDEX_POINTER));
      const auto path_id = deserializer.get_size_t(make_pointer(ptr, PATH_ID_POINTER));
      point_set.emplace_back(path_id, point_index);
    }
    joined_point_indices.push_back(point_set);
  }
  deserializer.register_reference_polisher(std::make_unique<ReferencePolisher>(joined_point_indices, *this));
}

void DisjointPathPointSetForest::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  const auto forest_ptr = make_pointer(root, FOREST_POINTER);
  serializer.start_array(m_forest.size(), forest_ptr);
  for (std::size_t i = 0; i < m_forest.size(); ++i) {
    const auto set_ptr = make_pointer(forest_ptr, i);
    serializer.start_array(m_forest[i].size(), set_ptr);
    std::size_t j = 0;
    for (auto* p : m_forest[i]) {
      const auto ptr = make_pointer(set_ptr, j);
      serializer.set_value(p->index(), make_pointer(ptr, INDEX_POINTER));
      serializer.set_value(p->path()->id(), make_pointer(ptr, PATH_ID_POINTER));
      j += 1;
    }
    serializer.end_array();
  }
  serializer.end_array();
}

}  // namespace omm
