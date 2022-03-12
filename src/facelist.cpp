#include "facelist.h"
#include "path/face.h"
#include "path/edge.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "serializers/deserializerworker.h"
#include "serializers/serializerworker.h"
#include "serializers/abstractdeserializer.h"
#include "transform.h"
#include "objects/pathobject.h"

namespace
{

static constexpr auto FACES_POINTER = "faces";
static constexpr auto PATH_ID_POINTER = "path";

}  // namespace

namespace omm
{

class FaceList::ReferencePolisher : public omm::serialization::ReferencePolisher
{
public:
  explicit ReferencePolisher(const std::size_t path_id)
      : m_path_id(path_id)
  {
  }

  void update_references(const std::map<std::size_t, AbstractPropertyOwner*>& map) override
  {
    const auto& path_object = dynamic_cast<const omm::PathObject&>(*map.at(m_path_id));
    const auto& path_vector = path_object.path_vector();
    for (std::size_t i = 0; i < m_faces.size(); ++i) {
      for (const auto& edge_repr : m_face_reprs.at(i)) {
        auto& p1 = path_vector.point_at_index(edge_repr.first);
        auto& p2 = path_vector.point_at_index(edge_repr.second);
        m_faces.at(i)->add_edge(Edge{p1, p2});
      }
    }
  }

  using EdgeRepr = std::pair<std::size_t, std::size_t>;
  using FaceRepr = std::deque<EdgeRepr>;

  FaceRepr& start_face(Face& face)
  {
    m_faces.emplace_back(&face);
    return m_face_reprs.emplace_back();
  }

private:
  const std::size_t m_path_id;
  std::deque<FaceRepr> m_face_reprs;
  std::deque<Face*> m_faces;
};

FaceList::FaceList() = default;
FaceList::~FaceList() = default;

FaceList::FaceList(const FaceList& other)
    : m_path_object(other.m_path_object)
    , m_faces(::copy(other.m_faces))
{
}

FaceList::FaceList(FaceList&& other) noexcept
{
  swap(*this, other);
}

FaceList& FaceList::operator=(FaceList other)
{
  swap(*this, other);
  return *this;
}

FaceList& FaceList::operator=(FaceList&& other) noexcept
{
  swap(*this, other);
  return *this;
}

void swap(FaceList& a, FaceList& b) noexcept
{
  std::swap(a.m_path_object, b.m_path_object);
  std::swap(a.m_faces, b.m_faces);
}

void FaceList::serialize(serialization::SerializerWorker& worker) const
{
  auto path_id_worker = worker.sub(PATH_ID_POINTER);
  if (m_path_object == nullptr) {
    path_id_worker->set_value(static_cast<std::size_t>(0));
  } else {
    path_id_worker->set_value(m_path_object->id());
    worker.sub(FACES_POINTER)->set_value(m_faces, [](const auto& f, auto& face_worker) {
      face_worker.set_value(f->edges(), [](const Edge& edge, auto& edge_worker) {
        edge_worker.set_value(std::vector{edge.a->index(), edge.b->index()});
      });
    });
  }
}

void FaceList::deserialize(serialization::DeserializerWorker& worker)
{
  m_faces.clear();
  m_path_object = nullptr;
  const auto path_id = worker.sub(PATH_ID_POINTER)->get<std::size_t>();
  if (path_id == 0) {
    return;
  }

  auto reference_polisher = std::make_unique<ReferencePolisher>(path_id);
  worker.sub(FACES_POINTER)->get_items([this, &reference_polisher](auto& face_worker) {
    auto& face_repr = reference_polisher->start_face(*m_faces.emplace_back(std::make_unique<Face>()));
    face_worker.get_items([&face_repr](auto& edge_worker) {
      const auto ids = edge_worker.template get<std::vector<std::size_t>>();
      if (ids.size() != 2) {
        throw serialization::AbstractDeserializer::DeserializeError("Expected two points per edge.");
      }
      face_repr.emplace_back(ids[0], ids[1]);
    });
  });

  worker.deserializer().register_reference_polisher(std::move(reference_polisher));
}

bool FaceList::operator==(const FaceList& other) const
{
  if (m_path_object != other.m_path_object || m_faces.size() != other.m_faces.size()) {
    return false;
  }

  for (std::size_t i = 0; i < m_faces.size(); ++i) {
    if (*m_faces[i] != *other.m_faces[i]) {
      return false;
    }
  }
  return true;
}

bool FaceList::operator!=(const FaceList& other) const
{
  return !(*this == other);
}

bool FaceList::operator<(const FaceList& other) const
{
  if (m_path_object == nullptr || other.m_path_object == nullptr) {
    return m_path_object < other.m_path_object;
  }

  if (m_path_object != other.m_path_object) {
    return m_path_object->id() < other.m_path_object->id();
  }

  if (m_faces.size() != other.m_faces.size()) {
    return m_faces.size() < other.m_faces.size();
  }

  for (std::size_t i = 0; i < m_faces.size(); ++i) {
    auto& face_i = *m_faces.at(i);
    auto& other_face_i = *other.m_faces.at(i);
    if (face_i != other_face_i) {
      return face_i < other_face_i;
    }
  }
  return false;  // face lists are equal
}

std::deque<Face> FaceList::faces() const
{
  return util::transform(m_faces, [](const auto& face) { return *face; });
}

}  // namespace omm
