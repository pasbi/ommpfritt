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
  (void) worker;
}

void FaceList::deserialize(serialization::DeserializerWorker& worker)
{
  (void) worker;
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
