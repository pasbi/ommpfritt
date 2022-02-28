#include "scene/disjointpathpointsetforest.h"
#include "serializers/abstractdeserializer.h"
#include "serializers/deserializerworker.h"
#include "serializers/serializerworker.h"
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

class DisjointPathPointSetForest::ReferencePolisher : public omm::serialization::ReferencePolisher
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

void DisjointPathPointSetForest::deserialize(serialization::DeserializerWorker& worker)
{
  std::deque<std::list<PathPointId>> joined_point_indices;
  worker.sub(FOREST_POINTER)->get_items([&joined_point_indices](auto& worker_i) {
    std::list<PathPointId> point_set;
    worker_i.get_items([&point_set](auto& worker_ii) {
      const auto point_index = worker_ii.sub(INDEX_POINTER)->get_size_t();
      const auto path_id = worker_ii.sub(PATH_ID_POINTER)->get_size_t();
      point_set.emplace_back(path_id, point_index);
    });
    joined_point_indices.push_back(point_set);
  });
  worker.deserializer().register_reference_polisher(std::make_unique<ReferencePolisher>(joined_point_indices, *this));
}

void DisjointPathPointSetForest::serialize(serialization::SerializerWorker& worker) const
{
  auto copy = *this;
  copy.remove_dangling_points();
  copy.serialize_impl(worker);
}

void DisjointPathPointSetForest::remove_dangling_points()
{
  remove_if(std::mem_fn(&PathPoint::is_dangling));
}

void DisjointPathPointSetForest::remove_if(const std::function<bool (const PathPoint*)>& predicate)
{
  for (auto& set : m_forest) {
    std::erase_if(set, predicate);
  }
  remove_empty_sets();
}

void DisjointPathPointSetForest::replace(const std::map<PathPoint*, PathPoint*>& dict)
{
  for (auto& old_set : m_forest) {
    Joint new_set;
    for (auto* old_point : old_set) {
      if (const auto it = dict.find(old_point); it != dict.end()) {
        new_set.insert(it->second);
      }
    }
    old_set = new_set;
  }
  remove_empty_sets();
}

void DisjointPathPointSetForest::serialize(serialization::SerializerWorker& worker, const Joint& joint)
{
  worker.set_value(joint, [](const auto* p, auto& worker_i) {
    worker_i.sub(INDEX_POINTER)->set_value(p->index());
    worker_i.sub(PATH_ID_POINTER)->set_value(p->path_vector()->path_object()->id());
  });
}

void DisjointPathPointSetForest::serialize_impl(serialization::SerializerWorker& worker) const
{
  worker.sub(FOREST_POINTER)->set_value(m_forest, [](const auto& joint, auto& worker_i) {
    serialize(worker_i, joint);
  });
}

}  // namespace omm
