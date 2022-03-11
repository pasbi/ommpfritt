#pragma once

#include <deque>
#include "memory"

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Face;
class PathObject;

class FaceList
{
public:
  explicit FaceList();
  ~FaceList();
  FaceList(const FaceList& other);
  FaceList(FaceList&& other) noexcept;
  FaceList& operator=(FaceList other);
  FaceList& operator=(FaceList&& other) noexcept;
  friend void swap(FaceList& a, FaceList& b) noexcept;
  class ReferencePolisher;
  void serialize(serialization::SerializerWorker& worker) const;
  void deserialize(serialization::DeserializerWorker& worker);
  [[nodiscard]] bool operator==(const FaceList& other) const;
  [[nodiscard]] bool operator!=(const FaceList& other) const;
  [[nodiscard]] bool operator<(const FaceList& other) const;

  PathObject* path_object() const;
  const std::deque<Face> faces() const;

private:
  PathObject* m_path_object = nullptr;
  std::deque<std::unique_ptr<Face>> m_faces;
};

}  // namespace
