#include "scene/disjointpathpointsetforest.h"
#include "serializers/abstractserializer.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/path.h"
#include "objects/pathobject.h"
#include "scene/scene.h"

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
        auto* path_object = dynamic_cast<PathObject*>(map.at(path_id));
        auto& path_point = path_object->geometry().point_at_index(point_index);
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
  auto copy = *this;
  copy.remove_dangling_points();
  copy.serialize_impl(serializer, root);
}

void DisjointPathPointSetForest::remove_dangling_points()
{
  for (auto& set : m_forest) {
    std::erase_if(set, [](const PathPoint* point) {
      if (point == nullptr) {
        return true;
      }
      if (point->path_vector() == nullptr) {
        return true;
      }
      const auto* path_object = point->path_vector()->path_object();
      if (path_object == nullptr) {
        return true;
      }
      if (path_object->scene() == nullptr) {
        return true;
      }
      return !path_object->scene()->contains(path_object);
    });
  }

  m_forest.erase(std::remove_if(m_forest.begin(), m_forest.end(), [](const auto& set) {
    return set.empty();
  }), m_forest.end());
}

void DisjointPathPointSetForest::replace(const std::map<PathPoint*, PathPoint*>& dict, bool keep_old)
{
  std::list<std::set<PathPoint*>> new_sets;
  for (auto& old_set : m_forest) {
    std::set<PathPoint*> new_set;
    for (auto* old_point : old_set) {
      if (const auto it = dict.find(old_point); it != dict.end()) {
        new_set.insert(it->second);
      }
    }
    if (keep_old) {
      new_sets.push_back(new_set);
    } else {
      old_set = new_set;
    }
  }

  m_forest.insert(m_forest.end(), new_sets.begin(), new_sets.end());
}

void DisjointPathPointSetForest::serialize_impl(AbstractSerializer& serializer, const Pointer& root) const
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
      serializer.set_value(p->path_vector()->path_object()->id(), make_pointer(ptr, PATH_ID_POINTER));
      j += 1;
    }
    serializer.end_array();
  }
  serializer.end_array();
}

}  // namespace omm
